#!/bin/bash

clear

pushd build || { echo "Making build directory" && mkdir build && cd build || exit ; }
cmake ..
make -j"$(nproc)"
popd || exit
