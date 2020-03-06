#!/bin/bash

clang-format-9 -i *.cpp *.hpp
pushd build
cmake ..
make -j$(nproc)
popd
