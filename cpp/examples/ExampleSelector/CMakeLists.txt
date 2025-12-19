add_test(ExampleSelector __init__)
add_test(ExampleSelector add)
# add_test(ExampleSelector drop)
add_custom_command(
        TARGET ExampleSelector_add POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/meta.json
                ${CMAKE_CURRENT_BINARY_DIR}/meta.json)