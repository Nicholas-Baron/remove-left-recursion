#!/bin/bash

for file in src/*.{cpp,hpp}; do
    echo "Formatting $file"
    clang-format-9 -i "$file"
done
clear
pushd build
cmake ..
make -j"$(nproc)"
popd
