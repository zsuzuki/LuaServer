#pragma once

#include <string>

namespace sol
{
struct state;
}

namespace LevelDB
{
///
void setup(sol::state& lua);

} // namespace LevelDB
