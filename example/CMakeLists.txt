cmake_minimum_required(VERSION 3.20)

project(xclient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(xpload REQUIRED)

add_executable(xclient xclient.cpp)
target_link_libraries(xclient xpload::xpload)

install(TARGETS xclient)
