add_test(TestBag __init__)
add_test(TestBag __str__)
add_test(TestBag insert)
add_test(TestBag remove)
add_test(TestBag remove_if)
add_test(TestBag size)
add_custom_command(
        TARGET TestBag_size POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/meta.json
                ${CMAKE_CURRENT_BINARY_DIR}/meta.json)