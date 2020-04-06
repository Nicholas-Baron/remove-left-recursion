cmake_minimum_required(VERSION 3.10)

if(NOT MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Og -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W2 /Za")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /O2")
endif()

project(remove_left_recurse CXX)

add_executable(check_grammar
        src/main.cpp
        src/grammar.cpp
        src/grammar_transform.cpp
    )

set_property(TARGET check_grammar PROPERTY CXX_STANDARD 17)
