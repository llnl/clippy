# Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy
# Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: MIT

#
# This function adds an mpi example.
#
# Works with 3.11 and tested through 3.15 (not tested yet)
cmake_minimum_required(VERSION 3.14)
set(ALLOW_DUPLICATE_CUSTOM_TARGETS TRUE)

include(FetchContent)

find_package(Threads)
# PP: added because I got an error "Could NOT find Threads (missing: Threads_FOUND)"
#     solution according to: https://github.com/alicevision/geogram/issues/2
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Boost 1.75 REQUIRED COMPONENTS)

#
#  Metall
find_package(Metall QUIET)
if (NOT Metall_FOUND)
    #set(METALL_WORK_DIR ${CMAKE_CURRENT_BINARY_DIR}/metall-work)
    set(METALL_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/metall-src)
    set(METALL_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/metall-build)
    set(METALL_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/metall-install)
		FetchContent_Declare(Metall
        GIT_REPOSITORY https://github.com/LLNL/metall.git
        GIT_TAG v0.26
				)
#        SOURCE_DIR ${METALL_SOURCE_DIR}
#        BINARY_DIR ${METALL_BUILD_DIR}
#        CMAKE_ARGS -DINSTALL_HEADER_ONLY=ON -DCMAKE_INSTALL_PREFIX=${METALL_INSTALL_DIR}
#        )
# set(METALL_INCLUDE_DIR ${METALL_INSTALL_DIR}/include)
  FetchContent_MakeAvailable(Metall)
endif ()
function ( add_example example_name )
  set(example_source "${example_name}.cpp")
  set(example_exe "${example_name}")
  add_executable(${example_exe} ${example_source})
  target_include_directories(${example_exe} PRIVATE ${Boost_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/../include)
  target_link_libraries(${example_exe} PRIVATE Metall)
  target_link_libraries(${example_exe} PRIVATE stdc++fs Threads::Threads)
  if (UNIX AND NOT APPLE)
    target_link_libraries(${example_exe} PRIVATE rt)
  endif ()
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})
  set(SOURCES ${example_source} ${CMAKE_SOURCE_DIR}/../include/clippy/clippy.hpp} ${CMAKE_SOURCE_DIR}/../include/experimental/cxx-compat.hpp)
endfunction()

add_example(grumpy)
add_example(sort_edges)
add_example(sort_strings)
add_example(howdy)
add_example(sum)
add_example(return_tuple)
add_example(sort_string_edges)

add_subdirectory(wordcounter)
add_subdirectory(dataframe)
add_subdirectory(dataframe-load)
add_subdirectory(oo-howdy)
add_subdirectory(oo-dataframe)
#add_subdirectory(mpi)
#add_subdirectory(ygm)
add_subdirectory(logic)
