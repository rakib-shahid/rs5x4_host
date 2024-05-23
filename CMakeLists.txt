cmake_minimum_required(VERSION 3.27.0)
project(rs5x4 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(test src/main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

find_package(fmt CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(hidapi CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(test PRIVATE cpr::cpr)