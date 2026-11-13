add_test(ExampleKeypath __init__)
add_test(ExampleKeypath add)
# add_test(ExampleKeypath drop)
add_custom_command(
        TARGET ExampleKeypath_add POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/meta.json
                ${CMAKE_CURRENT_BINARY_DIR}/meta.json)