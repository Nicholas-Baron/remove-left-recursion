cmake_minimum_required(VERSION 3.10)

message("Compiler is ${CMAKE_CXX_COMPILER}")
# Check whether we are compiling on Windows
# Any suggestions welcome on how to improve this line
if(NOT CMAKE_CXX_COMPILER MATCHES MSVC|cl[^a])
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Og -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O")
	message("Setting options to Unix style")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W2 /Za")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /O2")
	message("Setting options to Windows style")
endif()

project(remove_left_recurse CXX)

add_executable(check_grammar
        src/main.cpp
        src/grammar.cpp
        src/grammar_transform.cpp
    )

set_property(TARGET check_grammar PROPERTY CXX_STANDARD 17)
