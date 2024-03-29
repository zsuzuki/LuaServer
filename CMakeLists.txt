cmake_minimum_required(VERSION 3.11)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_definitions(-DLUA_USER_H=<luauser.h>)
if(WIN32)
set(CMAKE_CXX_FLAGS "-EHa")
add_definitions(-DBOOST_UUID_FORCE_AUTO_LINK)
else()
set(CMAKE_CXX_FLAGS "-g")
endif()

project(luaserver)

find_package(leveldb CONFIG REQUIRED)
find_package(cpprestsdk CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost 1.70.0 REQUIRED COMPONENTS chrono exception filesystem thread random system)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lua)
include_directories(${OPENSSL_INCLUDE_DIR})

link_directories(/usr/local/lib)

set(serv_src
    src/main.cpp
    src/server.cpp
    src/client.cpp
    src/leveldb.cpp
    src/async_command.cpp)

file(GLOB lua lua/*.c)
file(GLOB libsrc lib/*.cpp)
add_executable(luaserver ${serv_src} ${libsrc} ${lua})

if(WIN32)
set(libs
    cpprestsdk::cpprestsdk_brotli_internal
    wsock32
    ws2_32)
endif()

target_link_libraries(luaserver
    PRIVATE
        leveldb::leveldb
        ${Boost_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        cpprestsdk::cpprest
        cpprestsdk::cpprestsdk_zlib_internal
        ${libs})
