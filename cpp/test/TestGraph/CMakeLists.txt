set(CMAKE_BUILD_TYPE Debug)


add_test(TestGraph __init__)
add_test(TestGraph __str__)
# add_test(TestGraph assign)
# add_test(TestGraph dump)
add_test(TestGraph dump2)
add_test(TestGraph add_edge)
add_test(TestGraph add_node)
add_test(TestGraph nv)
add_test(TestGraph ne)
add_test(TestGraph degree)
add_test(TestGraph add_series)
add_test(TestGraph connected_components)
add_test(TestGraph drop_series)
add_test(TestGraph copy_series)
add_test(TestGraph series_str)
add_test(TestGraph extrema)
add_test(TestGraph count)
add_custom_command(
        TARGET TestGraph_nv POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/meta.json
                ${CMAKE_CURRENT_BINARY_DIR}/meta.json)
