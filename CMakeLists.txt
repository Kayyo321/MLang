cmake_minimum_required(VERSION 3.22)
project(Basic)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")

add_executable(Basic Main.cpp Lexer.cpp Lexer.h Token.h Parser.cpp Parser.h TinyExpr/TinyExpr.cpp TinyExpr/TinyExpr.h)