add_test(ExampleBag __init__)
add_test(ExampleBag __str__)
add_test(ExampleBag insert)
add_test(ExampleBag remove)
add_test(ExampleBag remove_if)
add_test(ExampleBag size)
add_custom_command(
        TARGET ExampleBag_size POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/meta.json
                ${CMAKE_CURRENT_BINARY_DIR}/meta.json)