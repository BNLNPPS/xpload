# From https://en.cppreference.com/w/cpp/filesystem:
#
# "Using this library may require additional compiler/linker options. GNU
# implementation prior to 9.1 requires linking with -lstdc++fs and LLVM
# implementation prior to LLVM 9.0 requires linking with -lc++fs"
#
# Here is what works. AppleClang does not seem to require a separate library
if ((${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "13") OR
    (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "9.1"))
  set(_std_cxx_filesystem_lib "stdc++fs")
endif()

add_executable(test_xpload test_xpload.cpp)
target_link_libraries(test_xpload xpload CURL::libcurl ${_std_cxx_filesystem_lib})
target_include_directories(test_xpload PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(test_xpload_rand test_xpload_rand.cpp)
target_link_libraries(test_xpload_rand xpload CURL::libcurl ${_std_cxx_filesystem_lib})
target_include_directories(test_xpload_rand PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_test(NAME "test_xpload_1" COMMAND bash -c "./test_xpload fetch -t tag_1")
add_test(NAME "test_xpload_2" COMMAND bash -c "./test_xpload fetch -t tag_1 -d domain_1")
add_test(NAME "test_xpload_3" COMMAND bash -c "./test_xpload fetch -t tag_1 -d domain_2")
# Should succeed when fails as expected
add_test(NAME "test_xpload_4" COMMAND bash -c "! ./test_xpload fetch -t tag_1 -d domain_2 -s 0")
add_test(NAME "test_xpload_5" COMMAND bash -c "./test_xpload fetch -t tag_1 -s 4")
add_test(NAME "test_xpload_6" COMMAND bash -c "./test_xpload fetch -t tag_2")
