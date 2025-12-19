# Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy
# Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: MIT


cmake_minimum_required(VERSION 3.26)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(ALLOW_DUPLICATE_CUSTOM_TARGETS TRUE)

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
cmake_policy(SET CMP0135 NEW)
endif()

project(CLIPPy
    VERSION 0.5
    DESCRIPTION "Command Line Interface Plus Python"
    LANGUAGES CXX)

include(FetchContent) 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
    # Let's ensure -std=c++xx instead of -std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Let's nicely support folders in IDE's
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Testing only available if this is the main app
    # Note this needs to be done in the main CMakeLists
    # since it calls enable_testing, which must be in the
    # main CMakeLists.
    # include(CTest)

    # Docs only available if this is the main app
    find_package(Doxygen)
    if(Doxygen_FOUND)
	#add_subdirectory(docs)
    else()
	message(STATUS "Doxygen not found, not building docs")
    endif()
endif()


#
# Boost
# Download and build Boost::json
set(BOOST_URL
    "https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-cmake.tar.gz"
    CACHE STRING "URL to fetch Boost tarball")


set(BOOST_INCLUDE_LIBRARIES json lexical_cast range)
set(BUILD_SHARED_LIBS ON)
FetchContent_Declare(
    Boost
    URL ${BOOST_URL})
FetchContent_MakeAvailable(Boost)


#
# JSONLogic
set(Boost_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/boost-install) # needed for jsonlogic

FetchContent_Declare(jsonlogic
    GIT_REPOSITORY https://github.com/LLNL/jsonlogic.git
    GIT_TAG v0.2.0
    SOURCE_SUBDIR cpp
)
# set(jsonlogic_INCLUDE_DIR ${jsonlogic_SOURCE_DIR}/cpp/include/jsonlogic)
FetchContent_MakeAvailable(jsonlogic)
message(STATUS "jsonlogic source dir: ${jsonlogic_SOURCE_DIR}")

    

### Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

include_directories("${PROJECT_SOURCE_DIR}/include")

option(TEST_WITH_SLURM "Run tests with Slurm" OFF)

# Testing & examples are only available if this is the main app
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message(STATUS "adding test subdir")
    add_subdirectory(examples)
    # Example codes are here.
    #add_subdirectory(examples)
endif()
