add_test(ExampleSet __init__)
add_test(ExampleSet __str__)
add_test(ExampleSet insert)
add_test(ExampleSet remove)
add_test(ExampleSet remove_if)
add_test(ExampleSet size)
add_custom_command(
        TARGET ExampleSet_size POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/meta.json
                ${CMAKE_CURRENT_BINARY_DIR}/meta.json)