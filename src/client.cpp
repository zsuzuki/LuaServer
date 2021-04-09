#include "util.hpp"
#include <atomic>
#include <atomic_queue.h>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <cpprest/http_listener.h>
#include <functional>
#include <iostream>
#include <json_util.hpp>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <queue>
#include <sol/sol.hpp>
#include <string>
#include <thread>

namespace Client
{
using namespace utility;           // Common utilities like string conversions
using namespace web;               // Common features like URIs.
using namespace web::http;         // Common HTTP functionality
using namespace web::http::client; // HTTP client features
using namespace concurrency::streams;              // Asynchronous streams
using namespace web::http::experimental::listener; // HTTP server features
using namespace Util;

namespace
{
sol::state* luaState;

///
///
///
struct Impl
{
  int         timeout     = 20;
  int         port        = 80;
  int         status_code = 0;
  std::string server;

  struct Result
  {
    sol::safe_function callback;
    nlohmann::json     response;
    pplx::task<void>   request;
  };

  Queue<Result> result_queue{4};

  Impl() = default;
  ~Impl()
  {
    while (auto q = result_queue.pop())
    {
      delete q;
    }
  }

  void request(std::string p, json::value qlist, sol::safe_function func)
  {
    http_client_config cfg;
    utility::seconds   to(timeout);
    cfg.set_timeout(to);
    std::string withport = "http://";
    withport += server + ":" + std::to_string(port);
    http_client client{STR(withport), cfg};
    uri_builder builder{STR(p)};
    auto        qlser = conversions::to_utf8string(qlist.serialize());
    auto        path  = conversions::to_utf8string(builder.to_string());

    auto result      = new Result;
    result->callback = func;
    result->request =
        client
            .request(methods::POST, path, qlser, utf8string("application/json"))
            .then([this](http_response response) {
              status_code = response.status_code();
              if (status_code == 200)
              {
                return response.extract_string();
              }
              return pplx::task_from_result(STR("{}"));
            })
            .then([result, this](utility::string_t str) {
              auto ret         = conversions::to_utf8string(str);
              result->response = nlohmann::json::parse(ret);
            })
            .then([=](pplx::task<void> previous_task) mutable {
              if (previous_task._GetImpl()->_HasUserException())
              {
                auto holder = previous_task._GetImpl()->_GetExceptionHolder();
                try
                {
                  holder->_RethrowUserException();
                }
                catch (std::exception& e)
                {
                  std::cerr << "http error:" << e.what() << std::endl;
                }
              }
              result_queue.push(result);
            });
  }

  //
  void query(std::string path, sol::safe_function func, sol::variadic_args va)
  {
    json::value ql;
    for (auto v : va)
    {
      if (v.is<sol::table>() == false)
        continue;
      sol::table t = v;
      for (auto& kvp : t)
      {
        auto& key = kvp.first;
        if (key.is<std::string>())
        {
          // keyはstringのみサポート
          ql[STR(key.as<std::string>())] = getValue(kvp.second);
        }
      }
    }
    request(path, ql, func);
  }

  //
  bool update()
  {
    if (auto req = result_queue.pop())
    {
      if (req->request.is_done())
      {
        sol::this_state ts(luaState->lua_state());
        auto            tbl = JSONUTIL::makeTable(ts, req->response);
        req->callback(tbl);
        delete req;
      }
      else
      {
        result_queue.push(req);
      }
    }
    return !result_queue.empty();
  }
};

} // namespace

//
void
setup(sol::state& lua)
{
  luaState = &lua;
  lua.new_usertype<Impl>("Client", "TimeOut", &Impl::timeout, "Port",
                         &Impl::port, "Server", &Impl::server, "StatusCode",
                         &Impl::status_code, "Query", &Impl::query, "Update",
                         &Impl::update);
}

} // namespace Client
