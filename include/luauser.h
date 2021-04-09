#pragma once

#define LUA_USE_LOCK false

#if LUA_USE_LOCK
#define lua_lock(L) LuaLock(L)
#define lua_unlock(L) LuaUnlock(L)
#define lua_userstateopen(L) LuaLockInitial(L)

struct lua_State;

void LuaLock(lua_State* L);
void LuaUnlock(lua_State* L);
#endif
