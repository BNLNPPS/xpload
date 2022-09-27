cmake_minimum_required(VERSION 3.20)

project(xpload VERSION 0.9.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set a debug postfix
set(CMAKE_DEBUG_POSTFIX "-dbg")

# Set installation destinations typical for *nix systems
include(GNUInstallDirs)

set(XPLOAD_INCLUDE_INSTALL_DIR        "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")
set(XPLOAD_RUNTIME_INSTALL_DIR        "${CMAKE_INSTALL_BINDIR}")
set(XPLOAD_LIBRARY_INSTALL_DIR        "${CMAKE_INSTALL_LIBDIR}")
set(XPLOAD_ARCHIVE_INSTALL_DIR        "${CMAKE_INSTALL_LIBDIR}")
set(XPLOAD_FRAMEWORK_INSTALL_DIR      "${CMAKE_INSTALL_LIBDIR}")
set(XPLOAD_DATA_INSTALL_DIR           "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")
set(XPLOAD_DATA_INSTALL_DIR_FULL      "${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}")
set(XPLOAD_CMAKE_CONFIG_INSTALL_DIR   "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
set(XPLOAD_CMAKE_CONFIG_TEMPLATE      "cmake/Config.cmake.in")
set(XPLOAD_CMAKE_CONFIG_DIR           "${CMAKE_CURRENT_BINARY_DIR}")
set(XPLOAD_CMAKE_VERSION_CONFIG_FILE  "${XPLOAD_CMAKE_CONFIG_DIR}/${PROJECT_NAME}ConfigVersion.cmake")
set(XPLOAD_CMAKE_PROJECT_CONFIG_FILE  "${XPLOAD_CMAKE_CONFIG_DIR}/${PROJECT_NAME}Config.cmake")

# Set up external dependencies
find_package(CURL REQUIRED)

add_subdirectory(src)
add_subdirectory(tools)

# Install files and export configs
include(CMakePackageConfigHelpers)

# Generate the config file that is includes the exports
configure_package_config_file(${XPLOAD_CMAKE_CONFIG_TEMPLATE}
    ${XPLOAD_CMAKE_PROJECT_CONFIG_FILE}
    INSTALL_DESTINATION ${XPLOAD_CMAKE_CONFIG_INSTALL_DIR}
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# Create and install version file
write_basic_package_version_file(${XPLOAD_CMAKE_VERSION_CONFIG_FILE}
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
    FILES ${XPLOAD_CMAKE_PROJECT_CONFIG_FILE} ${XPLOAD_CMAKE_VERSION_CONFIG_FILE}
    DESTINATION ${XPLOAD_CMAKE_CONFIG_INSTALL_DIR}
)

install(
    EXPORT xploadTargets
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${XPLOAD_CMAKE_CONFIG_INSTALL_DIR}
)

install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/config"
    DESTINATION ${XPLOAD_DATA_INSTALL_DIR}
)


# Set up tests
include(CTest)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
