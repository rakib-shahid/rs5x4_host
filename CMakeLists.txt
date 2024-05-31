cmake_minimum_required(VERSION 3.27.0)
project(rs5x4 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(test src/main.cpp)

# Ensure you use the correct package names and check for errors
# You can add error messages if packages are not found

find_package(cpr CONFIG REQUIRED)
if(NOT cpr_FOUND)
    message(FATAL_ERROR "cpr library not found. Please install cpr or set CMAKE_PREFIX_PATH appropriately.")
endif()

find_package(hidapi CONFIG REQUIRED)
if(NOT hidapi_FOUND)
    message(FATAL_ERROR "hidapi library not found. Please install hidapi or set CMAKE_PREFIX_PATH appropriately.")
endif()

find_package(nlohmann_json CONFIG REQUIRED)
if(NOT nlohmann_json_FOUND)
    message(FATAL_ERROR "nlohmann_json library not found. Please install nlohmann_json or set CMAKE_PREFIX_PATH appropriately.")
endif()

find_package(httplib CONFIG REQUIRED)
if(NOT httplib_FOUND)
    message(FATAL_ERROR "httplib library not found. Please install httplib or set CMAKE_PREFIX_PATH appropriately.")
endif()

find_package(Stb REQUIRED)
if(NOT Stb_FOUND)
    message(FATAL_ERROR "Stb library not found. Please install Stb or set CMAKE_PREFIX_PATH appropriately.")
endif()

# Include directories and link libraries
target_include_directories(test PRIVATE ${Stb_INCLUDE_DIR})
target_link_libraries(test PRIVATE cpr::cpr hidapi::hidapi nlohmann_json::nlohmann_json httplib::httplib)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
