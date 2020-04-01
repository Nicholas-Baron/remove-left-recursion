#!/bin/sh

[ -d tools ] || { echo "Could not find tools directory"; exit ; }
[ -d tests ] || { echo "Could not find tests directory"; exit ; }

tools/build.sh 2> build_log.txt || exit

[ "$(grep -ic error build_log.txt)" -ne 0 ] && echo "There are build errors" && exit

test_log="test_log.txt"

date > "$test_log"
for file in tests/*; do
    echo "$file" | tee -a "$test_log"
    build/check_grammar "$file" 2>&1 | tee -a "$test_log"
    echo ""
done
