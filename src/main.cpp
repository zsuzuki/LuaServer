#include "client.hpp"
#include "leveldb.hpp"
#include "server.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <json_util.hpp>
#include <sol/sol.hpp>
#include <string>
#include <vector>
#if !defined(_MSC_VER)
#include <dirent.h>
#include <unistd.h>
#else
#include <WinSock2.h>
#include <Windows.h>
#endif

namespace
{
///
sol::state LUA;

///
/// UUID生成
///
std::string
gen_uuid()
{
  auto uuid = boost::uuids::random_generator()();
  return boost::lexical_cast<std::string>(uuid);
}

///
/// ファイルリストの取得
///
sol::variadic_results
get_file_list(sol::this_state L, std::string dir)
{
  sol::variadic_results r;

  int        fidx  = 0;
  sol::table flist = LUA.create_table();
#if defined(_MSC_VER) // Windows
  HANDLE          hFind;
  WIN32_FIND_DATA win32fd;

  auto fdir = dir + "/*";
  hFind     = FindFirstFile(fdir.c_str(), &win32fd);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    do
    {
      if (!(win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      {
        std::string fname = win32fd.cFileName;
        flist[++fidx]     = fname;
      }
    } while (FindNextFile(hFind, &win32fd));

    FindClose(hFind);
  }
#else // UNIX系
  auto dp = opendir(dir.c_str());
  if (dp)
  {
    for (auto entry = readdir(dp); entry != nullptr; entry = readdir(dp))
    {
      if (entry->d_type != DT_DIR)
      {
        std::string fname = entry->d_name;
        flist[++fidx]     = fname;
      }
    }
  }
#endif
  r.push_back({L, sol::in_place_type<bool>, flist.size() > 0});
  r.push_back({L, sol::in_place, flist});
  return r;
}

///
/// カレントディレクトリの変更
///
bool
change_dir(std::string dir)
{
#if defined(_MSC_VER) // Windows
  return SetCurrentDirectory(dir.c_str());
#else
  return chdir(dir.c_str()) == 0;
#endif
}

} // namespace

///
/// main
///
int
main(int argc, char** argv)
{
  std::vector<std::string> l_args, args;
  for (int i = 1; i < argc; i++)
    l_args.emplace_back(argv[i]);

  std::string lua_file       = "";
  bool        direct_execute = false;
  std::string lua_script;
  // 引数取得
  for (auto& s : l_args)
  {
    if (direct_execute)
    {
      lua_script     = s;
      direct_execute = false;
    }
    else if (s == "-e")
      direct_execute = true;
    else if (lua_file.empty() && lua_script.empty())
      lua_file = s;
    else
      args.push_back(s);
  }
  if (lua_file.empty() && lua_script.empty())
  {
    std::cerr << "no file" << std::endl;
    return 1;
  }

  LUA.open_libraries(sol::lib::base, sol::lib::package, sol::lib::coroutine,
                     sol::lib::string, sol::lib::math, sol::lib::table,
                     sol::lib::debug, sol::lib::bit32, sol::lib::os,
                     sol::lib::io);

  try
  {
    LUA["Args"]        = args;
    LUA["ToJSON"]      = JSONUTIL::toJSON;
    LUA["FromJSON"]    = JSONUTIL::fromJSON;
    LUA["GetFileList"] = get_file_list;
    LUA["ChangeDir"]   = change_dir;
    LUA["UUID"]        = gen_uuid;

    Server::setup(LUA);
    Client::setup(LUA);
    LevelDB::setup(LUA);
    if (lua_script.empty() == false)
      LUA.script(lua_script);
    else
      LUA.script_file(lua_file);
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
