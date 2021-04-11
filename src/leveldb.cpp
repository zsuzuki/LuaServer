#include "leveldb.hpp"
#include <iostream>
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <sol/sol.hpp>

namespace LevelDB
{
namespace
{
leveldb::DB* db;

///
///
///
bool
start(const char* dbpath)
{
  leveldb::Options options;
  options.create_if_missing = true;
  auto status               = leveldb::DB::Open(options, dbpath, &db);
  if (!status.ok())
  {
    std::cerr << status.ToString() << std::endl;
    return false;
  }
  return true;
}

///
///
///
bool
put(std::string key, std::string value)
{
  auto s = db->Put(leveldb::WriteOptions(), key, value);
  if (!s.ok())
  {
    std::cerr << s.ToString() << std::endl;
    return false;
  }
  return true;
}

///
///
///
sol::variadic_results
get(sol::this_state L, std::string key)
{
  sol::variadic_results r;
  std::string           value;
  auto                  s = db->Get(leveldb::ReadOptions(), key, &value);
  r.push_back({L, sol::in_place_type<std::string>, value});
  r.push_back({L, sol::in_place_type<bool>, s.ok()});
  return r;
}
} // namespace

///
///
///
void
setup(sol::state& lua)
{
  sol::table ldb = lua.create_table();
  ldb["Start"]   = start;
  ldb["Put"]     = put;
  ldb["Get"]     = get;
  lua["LevelDB"] = ldb;
}

} // namespace LevelDB
