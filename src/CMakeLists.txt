configure_file(config.h.in config.h @ONLY)
configure_file(version.h.in version.h @ONLY)

set(source_files
    configurator.cpp
    fetch.cpp
    push.cpp
    requests.cpp
)

add_library(xpload ${source_files})
target_link_libraries(xpload PUBLIC CURL::libcurl)

target_include_directories(xpload
    INTERFACE
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/contrib
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(xpload PROPERTIES
    VERSION ${PROJECT_VERSION}
    POSITION_INDEPENDENT_CODE ON)

install(TARGETS xpload EXPORT xploadTargets
    RUNTIME   DESTINATION ${XPLOAD_RUNTIME_INSTALL_DIR}
    LIBRARY   DESTINATION ${XPLOAD_LIBRARY_INSTALL_DIR}
    ARCHIVE   DESTINATION ${XPLOAD_ARCHIVE_INSTALL_DIR}
    FRAMEWORK DESTINATION ${XPLOAD_FRAMEWORK_INSTALL_DIR})

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/xpload/ DESTINATION ${XPLOAD_INCLUDE_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config.h DESTINATION ${XPLOAD_INCLUDE_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/version.h DESTINATION ${XPLOAD_INCLUDE_INSTALL_DIR})
