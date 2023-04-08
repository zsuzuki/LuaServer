#pragma once

#include <cpprest/http_client.h>
#include <cpprest/http_listener.h>
#include <sol/sol.hpp>

namespace Util
{
using namespace utility;
using namespace web;

//
inline utility::string_t
STR(const std::string str)
{
  return conversions::to_string_t(str);
}

///
/// 引数用JSON生成
///
template <class T>
json::value
getValue(T& v)
{
  auto get = [](auto gv)
  {
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

/// JSONテーブル化
inline sol::table
buildTable(sol::state& lua, json::value& j)
{
  sol::table tbl = lua.create_table();
  if (j.is_null())
    return tbl;
  if (j.is_array())
  {
    json::array& ar = j.as_array();
    int          i  = 0;
    for (auto& v : ar)
    {
      tbl[i++] = v.as_double();
    }
  }
  else if (j.is_object())
  {
    json::object obj = j.as_object();
    for (auto& o : obj)
    {
      auto& value = o.second;
      auto* name  = o.first.c_str();
      if (value.is_integer())
        tbl[name] = value.as_integer();
      else if (value.is_double())
        tbl[name] = value.as_double();
      else if (value.is_boolean())
        tbl[name] = value.as_bool();
      else if (value.is_string())
        tbl[name] = value.as_string();
      else if (value.is_object() || value.is_array())
        tbl[name] = buildTable(lua, value);
    }
  }
  return tbl;
}

} // namespace Util
