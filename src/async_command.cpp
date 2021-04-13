#include "async_command.hpp"
#include <boost/process.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sol/sol.hpp>
#include <string>

namespace AsyncCommand
{
namespace process = boost::process;
using child_ptr   = std::shared_ptr<process::child>;

namespace
{
sol::state* luaState = nullptr;

///
///
///
class Impl
{
  child_ptr child;

public:
  Impl()  = default;
  ~Impl() = default;

  bool execute(std::string cmd)
  {
    child = std::make_shared<process::child>(cmd);
    return true;
  }
  bool is_execute() { return child->running(); }

  sol::variadic_results get_result(sol::this_state L) const
  {
    sol::variadic_results r;
    if (child->running() == false)
    {
      r.push_back({L, sol::in_place_type<bool>, true});
      r.push_back({L, sol::in_place_type<int>, child->exit_code()});
    }
    else
    {
      r.push_back({L, sol::in_place_type<bool>, false});
      r.push_back({L, sol::in_place_type<int>, 0});
    }
    return r;
  }
  void done()
  {
    if (child)
    {
      child->join();
      child->terminate();
      child.reset();
    }
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
                         &Impl::is_execute, "GetResult", &Impl::get_result,
                         "Done", &Impl::done);
}

} // namespace AsyncCommand
