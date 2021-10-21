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

include(CMakePackageConfigHelpers)

# Generate config file that includes the exported targets
set(config_file "${CMAKE_CURRENT_BINARY_DIR}/xploadConfig.cmake")
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    ${config_file}
    INSTALL_DESTINATION ${XPLOAD_CMAKE_CONFIG_INSTALL_DIR}
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# Create and install version file
set(version_file "${CMAKE_CURRENT_BINARY_DIR}/xploadConfigVersion.cmake")
write_basic_package_version_file(${version_file}
    VERSION ${XPLOAD_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES "${config_file}" "${version_file}" DESTINATION ${XPLOAD_CMAKE_CONFIG_INSTALL_DIR})

# Set up tests
include(CTest)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
