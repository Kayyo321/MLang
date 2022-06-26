cmake_minimum_required(VERSION 3.22)
project(Basic)

set(CMAKE_CXX_STANDARD 20)

add_executable(Basic Main.cpp Lexer.cpp Lexer.h Token.h Parser.cpp Parser.h)
