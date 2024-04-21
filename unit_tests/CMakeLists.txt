aux_source_directory(. SRC_LIST)

add_executable(vector_test ${SRC_LIST})

target_link_libraries(vector_test PRIVATE ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} Vector)

gtest_discover_tests(vector_test)