cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(xpload LANGUAGES CXX)

set(XPLOAD_VERSION_MAJOR 0 CACHE STRING "major version of xpload" FORCE)
set(XPLOAD_VERSION_MINOR 0 CACHE STRING "minor version of xpload" FORCE)
set(XPLOAD_VERSION_PATCH 0 CACHE STRING "patch version of xpload" FORCE)
set(XPLOAD_VERSION "${XPLOAD_VERSION_MAJOR}.${XPLOAD_VERSION_MINOR}.${XPLOAD_VERSION_PATCH}" CACHE STRING "version of xpload" FORCE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set a debug postfix
set(CMAKE_DEBUG_POSTFIX "-dbg")

# Set installation destinations typical for *nix systems
include(GNUInstallDirs)

set(XPLOAD_INC_INSTALL_DIR          "${CMAKE_INSTALL_INCLUDEDIR}/xpload")
set(XPLOAD_RUNTIME_INSTALL_DIR      "${CMAKE_INSTALL_BINDIR}")
set(XPLOAD_LIBRARY_INSTALL_DIR      "${CMAKE_INSTALL_LIBDIR}")
set(XPLOAD_ARCHIVE_INSTALL_DIR      "${CMAKE_INSTALL_LIBDIR}")
set(XPLOAD_FRAMEWORK_INSTALL_DIR    "${CMAKE_INSTALL_LIBDIR}")
set(XPLOAD_DATA_INSTALL_DIR         "${CMAKE_INSTALL_DATADIR}/xpload-${XPLOAD_VERSION}")
set(XPLOAD_CMAKE_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/xpload-${XPLOAD_VERSION}/cmake")

# Set up external dependencies
find_package(CURL REQUIRED)
find_package(jsoncpp REQUIRED)

add_subdirectory(src)

# Install additional files such as README, LICENSE, etc.
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.md" DESTINATION ${XPLOAD_DATA_INSTALL_DIR})
install(EXPORT xploadTargets DESTINATION ${XPLOAD_CMAKE_CONFIG_INSTALL_DIR} FILE xpload-config.cmake)

# Set up tests
include(CTest)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
