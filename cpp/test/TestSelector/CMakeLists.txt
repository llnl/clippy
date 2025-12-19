add_test(TestSelector __init__)
add_test(TestSelector add)
# add_test(TestSelector drop)
add_custom_command(
        TARGET TestSelector_add POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/meta.json
                ${CMAKE_CURRENT_BINARY_DIR}/meta.json)