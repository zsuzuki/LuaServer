#include <iostream>
#include <lua.hpp>
#include <mutex>
#include <thread>

namespace
{
std::mutex luaLock;
}

extern "C"
{
  void LuaLock(lua_State* L) { luaLock.lock(); }
  void LuaUnlock(lua_State* L) { luaLock.unlock(); }
}
