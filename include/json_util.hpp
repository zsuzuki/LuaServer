#include <nlohmann/json.hpp>
#include <sol/sol.hpp>
#include <string>

namespace JSONUTIL
{
std::string           toJSON(sol::table tbl);
sol::variadic_results fromJSON(sol::this_state L, std::string s);
sol::table            makeTable(sol::this_state L, nlohmann::json& json);
} // namespace JSONUTIL
