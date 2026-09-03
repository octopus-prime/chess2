cmake_minimum_required(VERSION 4.2)
project(HelloWorld CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
  -march=native
  -Wall
  -Wextra
  -Wpedantic
  -Wconversion
  -Wsign-conversion
  -Wshadow
  -Wimplicit-fallthrough
  -Wunreachable-code
  -Wmissing-field-initializers
  -Wundef
  # -Werror
)

add_executable(chess
  src/move_generator.cpp
  src/move_executor.cpp
  src/chess.cpp
)

target_include_directories(chess PRIVATE src)
