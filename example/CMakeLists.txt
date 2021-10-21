cmake_minimum_required(VERSION 3.20)

project(xclient LANGUAGES CXX)

find_package(xpload REQUIRED)

add_executable(xclient xclient.cpp)
target_link_libraries(xclient xpload::xpload)
