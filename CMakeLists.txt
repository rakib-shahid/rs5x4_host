cmake_minimum_required(VERSION 3.10)

project(rs5x4_test)

find_package(fmt CONFIG REQUIRED)
find_package(hidapi CONFIG REQUIRED)

add_executable(test src/main.cpp)

target_link_libraries(test PRIVATE fmt::fmt hidapi::hidapi)