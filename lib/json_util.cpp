#include <json_util.hpp>

namespace JSONUTIL
{

using JSON = nlohmann::json;

//
// tableからJSON(文字列)
//
namespace
{
void
set_val(JSON& j, sol::object& key, JSON v)
{
  if (key.is<std::string>())
    j[key.as<std::string>()] = v;
  else
    j[key.as<int>() - 1] = v;
}

//
JSON
getloop(sol::table t)
{
  JSON j;
  for (auto& kvp : t)
  {
    auto v = kvp.second;
    if (v.is<bool>())
      set_val(j, kvp.first, v.as<bool>());
    else if (v.is<double>())
      set_val(j, kvp.first, v.as<double>());
    else if (v.is<std::string>())
      set_val(j, kvp.first, v.as<std::string>());
    else if (v.is<sol::table>())
      set_val(j, kvp.first, getloop(v.as<sol::table>()));
  }
  return j;
};
} // namespace

//
std::string
toJSON(sol::table tbl)
{
  return getloop(tbl).dump();
}

//
// 文字列(JSON)からtable
//
namespace
{
sol::table
get_table(sol::state_view& lua, JSON& j)
{
  sol::table table    = lua.create_table();
  bool       is_array = j.is_array();
  int        idx      = 0;
  for (auto& el : j.items())
  {
    auto set = [&](auto v)
    {
      if (is_array)
        table[++idx] = v;
      else
        table[el.key()] = v;
    };
    auto val = el.value();
    if (val.is_string())
      set(val.get<std::string>());
    else if (val.is_boolean())
      set(val.get<bool>());
    else if (val.is_number())
      set(val.get<double>());
    else if (val.is_array())
      set(get_table(lua, val));
    else if (val.is_object())
      set(get_table(lua, val));
  }
  return table;
}
} // namespace

//
sol::variadic_results
fromJSON(sol::this_state L, std::string s)
{
  sol::variadic_results r;

  JSON j   = JSON::parse(s);
  auto lua = sol::state_view{L};

  r.push_back({L, sol::in_place, get_table(lua, j)});
  return r;
}

//
sol::table
makeTable(sol::this_state L, nlohmann::json& json)
{
  auto lua = sol::state_view{L};
  return get_table(lua, json);
}

} // namespace JSONUTIL
