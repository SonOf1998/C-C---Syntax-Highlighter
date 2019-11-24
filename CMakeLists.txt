cmake_minimum_required(VERSION 3.14)
project(cpphf2)

set(CMAKE_CXX_STANDARD 17)

add_executable(cpphf2 main.cpp CSrcHighlighter.cpp CSrcHighlighter.h CppSrcHighlighter.cpp CppSrcHighlighter.h)