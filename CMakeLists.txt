cmake_minimum_required(VERSION 3.10)

project(remove_left_recurse)

add_executable(check_grammar
        src/main.cpp
        src/grammar.cpp
        src/grammar_transform.cpp
    )

set_property(TARGET check_grammar PROPERTY CXX_STANDARD 17)
target_compile_options(check_grammar PRIVATE -Wall -Wextra)
