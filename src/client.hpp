#pragma once

namespace sol
{
struct state;
}

namespace Client
{
/// クライアント機能の初期化
void setup(sol::state& lua);
} // namespace Client
