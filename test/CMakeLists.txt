add_executable(test_xpload test_xpload.cpp)
target_link_libraries(test_xpload xpload CURL::libcurl)
target_include_directories(test_xpload PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(test_xpload_rand test_xpload_rand.cpp)
target_link_libraries(test_xpload_rand xpload CURL::libcurl)
target_include_directories(test_xpload_rand PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_test(NAME "test_xpload" COMMAND bash -c "XPLOAD_CONFIG=dev ./test_xpload -t example_tag_1")
