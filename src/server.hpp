#pragma once

namespace sol
{
struct state;
}

namespace Server
{
/// サーバー機能の初期化
void setup(sol::state& lua);
} // namespace Server
