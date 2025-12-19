#!/bin/sh

PROJ_ROOT_DIR=$(pwd)/..
CPP_BUILD_DIR=$PROJ_ROOT_DIR/cpp/build
CLIPPY_BACKEND_PATH=$CPP_BUILD_DIR/examples pytest
