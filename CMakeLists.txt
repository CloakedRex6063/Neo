cmake_minimum_required(VERSION 3.20)
project(Neo)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0077 NEW)

include(FetchContent)

option(WITH_SANDBOX "Copy the test sandbox project" ON)

add_subdirectory(Engine)
add_subdirectory(Editor)

