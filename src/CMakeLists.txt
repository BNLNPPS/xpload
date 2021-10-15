configure_file(config.h.in config.h @ONLY)
configure_file(version.h.in version.h @ONLY)

set(source_files
    configurator.cpp
)

add_library(xpload ${source_files})
target_link_libraries(xpload jsoncpp_lib)

target_include_directories(xpload
    INTERFACE
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(xpload PROPERTIES
    VERSION ${XPLOAD_VERSION}
    POSITION_INDEPENDENT_CODE ON)

install(TARGETS xpload EXPORT xploadTargets
    RUNTIME   DESTINATION ${XPLOAD_RUNTIME_INSTALL_DIR}
    LIBRARY   DESTINATION ${XPLOAD_LIBRARY_INSTALL_DIR}
    ARCHIVE   DESTINATION ${XPLOAD_ARCHIVE_INSTALL_DIR}
    FRAMEWORK DESTINATION ${XPLOAD_FRAMEWORK_INSTALL_DIR})

# Create and install version file
include(CMakePackageConfigHelpers)
set(version_file "${CMAKE_CURRENT_BINARY_DIR}/cmake/xpload-config-version.cmake")
write_basic_package_version_file(${version_file}
    VERSION ${XPLOAD_VERSION}
    COMPATIBILITY AnyNewerVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config.h DESTINATION ${XPLOAD_INC_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/version.h DESTINATION ${XPLOAD_INC_INSTALL_DIR})
