set(CMAKE_BUILD_TYPE Debug)


add_test(ExampleGraph __init__)
add_test(ExampleGraph __str__)
# add_test(ExampleGraph assign)
# add_test(ExampleGraph dump)
add_test(ExampleGraph dump2)
add_test(ExampleGraph add_edge)
add_test(ExampleGraph add_node)
add_test(ExampleGraph nv)
add_test(ExampleGraph ne)
add_test(ExampleGraph degree)
add_test(ExampleGraph add_series)
add_test(ExampleGraph connected_components)
add_test(ExampleGraph drop_series)
add_test(ExampleGraph copy_series)
add_test(ExampleGraph series_str)
add_test(ExampleGraph extrema)
add_test(ExampleGraph count)
add_custom_command(
        TARGET ExampleGraph_nv POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/meta.json
                ${CMAKE_CURRENT_BINARY_DIR}/meta.json)
