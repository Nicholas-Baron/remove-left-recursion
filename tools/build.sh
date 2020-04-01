#!/bin/bash

clang-format-9 -i ./*.cpp ./*.hpp
pushd build || { echo "Making build directory" && mkdir build && cd build || exit ; }
cmake ..
make -j"$(nproc)"
popd || exit
