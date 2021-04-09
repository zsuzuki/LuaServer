#include "server.hpp"
#include "util.hpp"
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <cpprest/http_listener.h>
#include <functional>
#include <iostream>
#include <map>
#include <queue>
#include <sol/sol.hpp>
#include <string>
#include <thread>

namespace Server
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

http_listener listener;
std::mutex    mutex;

// 返答用(GET)
struct GetRes
{
  using unique_lock = std::unique_lock<std::mutex>;
  json::value                        result;
  std::map<std::string, std::string> arguments;
  std::condition_variable            cond;
  std::mutex                         mutex;
  unique_lock                        lock{mutex};
};
std::queue<GetRes*> queueGet;
// 返答用(POST)
struct PostRes
{
  using unique_lock = std::unique_lock<std::mutex>;
  json::value             result;
  json::value             arguments;
  std::condition_variable cond;
  std::mutex              mutex;
  unique_lock             lock{mutex};
};
std::queue<PostRes*> queuePost;

const json::value emptyStr = json::value::string(STR("empty"));

//
void
empty_response()
{
  std::cout << "emtpry response" << std::endl;
}

// GETはあらかじめセットされたJSONを返すだけ
void
respond_get(http_request req)
{
  std::lock_guard<std::mutex> lock(mutex);

  auto& lua = *luaState;

  GetRes r;

  auto getVars = uri::split_query(req.request_uri().query());
  for (auto& p : getVars)
  {
    auto key         = conversions::to_utf8string(p.first);
    auto value       = conversions::to_utf8string(p.second);
    r.arguments[key] = value;
  }

  queueGet.push(&r);
  r.cond.wait(r.lock);

  if (r.result.is_null())
    req.reply(status_codes::BadRequest, emptyStr);
  else
    req.reply(status_codes::OK, r.result);
}

// POSTに対する返答
void
respond_post(http_request req)
{
  req.extract_json()
      .then([&](pplx::task<json::value> task) {
        try
        {
          PostRes r;
          r.arguments = task.get();
          queuePost.push(&r);
          r.cond.wait(r.lock);

          if (r.result.is_null())
            req.reply(status_codes::BadRequest, emptyStr);
          else
            req.reply(status_codes::OK, r.result);
        }
        catch (http_exception const& e)
        {
          std::cout << e.what() << std::endl;
        }
      })
      .wait();
}

// サーバー開始
void
start(std::string pt, int pr, std::string domain)
{
  auto& lua = *luaState;

  char hn[1024];
  gethostname(hn, sizeof(hn));
  struct hostent* he = gethostbyname(hn);
  if (!he)
  {
    std::cerr << "Bad host lookup" << std::endl;
    return;
  }
  std::string hostname = hn;
  for (int i = 0; he->h_addr_list[i] != 0; ++i)
  {
    struct in_addr addr;
    memcpy(&addr, he->h_addr_list[i], sizeof(struct in_addr));
    // std::cout << "Address " << i << ": " << inet_ntoa(addr) << std::endl;
    hostname = inet_ntoa(addr);
  }
  if (!domain.empty())
    hostname += domain;
  lua["Server"]["hostname"] = hostname;
  lua["Server"]["port"]     = pr;
  lua["Server"]["path"]     = pt;

  auto url = std::string("http://") + hostname;
  auto n   = url + ":" + std::to_string(pr) + pt;
  std::cout << "URL: " << n << std::endl;
  http_listener tmp(STR(n));
  listener = std::move(tmp);
  listener.open().wait();
  listener.support(methods::GET, [&](http_request req) { respond_get(req); });
  listener.support(methods::POST, [&](http_request req) { respond_post(req); });
}

// サーバー停止
void
stop()
{
  listener.close();
}

//
void
update()
{
  auto& lua = *luaState;
  // GETリクエストへの返答生成処理
  if (!queueGet.empty())
  {
    auto*      r    = queueGet.front();
    sol::table args = lua.create_table();
    for (auto& v : r->arguments)
      args[v.first] = v.second;
    auto       res = lua["Server"]["MakeGetResponse"](args);
    sol::table t   = res[0];
    for (auto& kvp : t)
    {
      auto& key = kvp.first;
      if (key.is<std::string>())
      {
        // keyはstringのみサポート
        r->result[STR(key.as<std::string>())] = getValue(kvp.second);
      }
    }
    r->cond.notify_one();
    queueGet.pop();
  }
  // POSTリクエストへの返答生成処理
  if (!queuePost.empty())
  {
    auto*      r    = queuePost.front();
    auto       args = buildTable(lua, r->arguments);
    auto       res  = lua["Server"]["MakePostResponse"](args);
    sol::table t    = res[0];
    for (auto& kvp : t)
    {
      auto& key = kvp.first;
      if (key.is<std::string>())
      {
        // keyはstringのみサポート
        r->result[STR(key.as<std::string>())] = getValue(kvp.second);
      }
    }
    r->cond.notify_one();
    queuePost.pop();
  }
}
} // namespace

///
void
setup(sol::state& lua)
{
  luaState                   = &lua;
  sol::table iflist          = lua.create_table();
  iflist["Start"]            = &start;
  iflist["Stop"]             = &stop;
  iflist["Update"]           = &update;
  iflist["MakeGetResponse"]  = &empty_response;
  iflist["MakePostResponse"] = &empty_response;
  iflist["Response"]         = lua.create_table();
  lua["Server"]              = iflist;
}

} // namespace Server
