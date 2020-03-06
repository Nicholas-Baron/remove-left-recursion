#!/bin/bash

clang-format-9 -i *.cpp *.hpp
pushd build
make -j$(nproc)
popd
