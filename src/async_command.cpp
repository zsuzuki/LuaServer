#include "async_command.hpp"
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <future>
#include <iostream>
#include <sol/sol.hpp>
#include <string>
#include <thread>

namespace AsyncCommand
{

namespace
{
sol::state* luaState = nullptr;

///
///
///
class Impl
{
  std::future<int> cmdret;
  std::atomic_bool hasret;
  int              retval;

public:
  Impl()  = default;
  ~Impl() = default;

  bool execute(std::string cmd)
  {
    hasret = false;
    cmdret = std::async(std::launch::async, [cmd] {
      errno = 0;
      return system(cmd.c_str());
    });
    return true;
  }
  bool is_execute()
  {
    bool valid = cmdret.valid();
    if (valid)
    {
      auto st   = cmdret.wait_for(std::chrono::seconds(0));
      auto exec = st != std::future_status::ready;
      if (!exec)
      {
        retval = cmdret.get();
        hasret = true;
      }
    }
    return valid;
  }

  sol::variadic_results get_result(sol::this_state L) const
  {
    sol::variadic_results r;
    if (hasret)
    {
      r.push_back({L, sol::in_place_type<bool>, true});
      r.push_back({L, sol::in_place_type<int>, retval});
    }
    else
    {
      r.push_back({L, sol::in_place_type<bool>, false});
      r.push_back({L, sol::in_place_type<int>, 0});
    }
    return r;
  }
};

} // namespace

///
///
///
void
setup(sol::state& lua)
{
  luaState = &lua;
  lua.new_usertype<Impl>("Execute", "Execute", &Impl::execute, "IsExecute",
                         &Impl::is_execute, "GetResult", &Impl::get_result);
}

} // namespace AsyncCommand
