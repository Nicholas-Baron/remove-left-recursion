cmake_minimum_required(VERSION 3.10)

project(remove_left_recurse)

add_executable(check_grammar
    main.cpp
    grammar.cpp
    )

set_property(TARGET check_grammar PROPERTY CXX_STANDARD 14)
