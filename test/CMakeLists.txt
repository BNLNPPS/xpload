add_executable(test_xpload test_xpload.cpp)
target_link_libraries(test_xpload xpload CURL::libcurl)
target_include_directories(test_xpload PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(test_xpload_rand test_xpload_rand.cpp)
target_link_libraries(test_xpload_rand xpload CURL::libcurl)
target_include_directories(test_xpload_rand PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_test(NAME "test_xpload_1" COMMAND bash -c "./test_xpload -t tag_1")
add_test(NAME "test_xpload_2" COMMAND bash -c "./test_xpload -t tag_1 -d domain_1")
add_test(NAME "test_xpload_3" COMMAND bash -c "./test_xpload -t tag_1 -d domain_2")
# Should succeed when fails as expected
add_test(NAME "test_xpload_4" COMMAND bash -c "! ./test_xpload -t tag_1 -d domain_2 -s 0")
add_test(NAME "test_xpload_5" COMMAND bash -c "./test_xpload -t tag_1 -s 4")
add_test(NAME "test_xpload_6" COMMAND bash -c "./test_xpload -t tag_2")
