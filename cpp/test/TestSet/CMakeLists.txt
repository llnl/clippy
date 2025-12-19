add_test(TestSet __init__)
add_test(TestSet __str__)
add_test(TestSet insert)
add_test(TestSet remove)
add_test(TestSet remove_if)
add_test(TestSet size)
add_custom_command(
        TARGET TestSet_size POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/meta.json
                ${CMAKE_CURRENT_BINARY_DIR}/meta.json)