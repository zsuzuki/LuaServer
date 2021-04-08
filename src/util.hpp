#pragma once

#include <cpprest/http_client.h>
#include <cpprest/http_listener.h>
#include <sol/sol.hpp>

namespace Util
{
using namespace utility;
using namespace web;

//
utility::string_t
STR(const std::string str)
{
  return conversions::to_string_t(str);
}

///
/// 引数生成
///
template <class T>
json::value
getValue(T& v)
{
  auto get = [](auto gv) {
    json::value r;
    bool        s = true;
    if (gv.template is<bool>())
      r = json::value::boolean(gv.template as<bool>());
    else if (gv.template is<double>())
      r = json::value::number(gv.template as<double>());
    else if (gv.template is<int64_t>())
      r = json::value::number(gv.template as<int64_t>());
    else if (gv.template is<std::string>())
      r = json::value::string(STR(gv.template as<std::string>()));
    else
      s = false;
    return std::make_pair(r, s);
  };

  auto pv = get(v);
  if (pv.second)
  {
    // プリミティブ型
    return pv.first;
  }

  json::value ret;
  if (v.template is<sol::table>())
  {
    bool init     = false;
    bool is_array = false;

    std::vector<json::value> varray;
    for (auto& kvp : v.template as<sol::table>())
    {
      auto& key = kvp.first;
      if (key.template is<std::string>())
      {
        if (init && is_array)
          continue;
        auto gr = get(kvp.second);
        if (gr.second)
          ret[STR(key.template as<std::string>())] = gr.first;
        else
          ret[STR(key.template as<std::string>())] = getValue(kvp.second);
      }
      else if (key.template is<int>())
      {
        if (init && is_array == false)
          continue;
        size_t index = key.template as<int>() - 1;
        if (index >= varray.size())
        {
          varray.resize(index + 1);
        }
        auto gr = get(kvp.second);
        if (gr.second)
          varray[index] = gr.first;
        else
          varray[index] = getValue(kvp.second);
        is_array = true;
      }
      init = true;
    }
    if (is_array)
      ret = json::value::array(varray);
  }
  return ret;
}
} // namespace Util
