cmake_minimum_required(VERSION 3.7.3)
project(huffman)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
	src/main.cpp
	)

add_executable(huffman ${SOURCES})
target_link_libraries(huffman PRIVATE m pthread)
