#!/bin/sh

./build.sh || exit

for file in tests/*; do
    echo "$file"
    build/check_grammar "$file"
    echo ""
done
