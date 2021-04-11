cmake_minimum_required(VERSION 3.11)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-g")
add_definitions(-DLUA_USER_H=<luauser.h>)

project(luaserver)

find_package(cpprestsdk CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost 1.70.0 REQUIRED COMPONENTS chrono thread system)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lua)
include_directories(${OPENSSL_INCLUDE_DIR})

set(serv_src src/main.cpp src/server.cpp src/client.cpp)

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
        ${Boost_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        cpprestsdk::cpprest
        cpprestsdk::cpprestsdk_zlib_internal
        ${libs})
