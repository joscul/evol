
cmake_minimum_required(VERSION 3.10)
project(evol)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Debug)

find_package(TBB REQUIRED)

file(GLOB MY_SRC "src/*.cpp")

# Add executable and link Lua and MySQL Connector/C libraries
add_executable(evol ${MY_SRC})
target_link_libraries(evol PRIVATE TBB::tbb)

