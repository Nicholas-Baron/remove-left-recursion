#!/bin/sh

./build.sh

for file in tests/*; do
    echo "$file"
    build/check_grammar "$file"
    echo ""
done
