#!/bin/sh

[ -d tools ] || { echo "Could not find tools directory"; exit ; }
[ -d tests ] || { echo "Could not find tests directory"; exit ; }

tools/build.sh || exit

for file in tests/*; do
    echo "$file"
    build/check_grammar "$file"
    echo ""
done
