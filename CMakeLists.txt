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
  # -Werror
  -Wconversion
  -Wsign-conversion
  -Wshadow
  -Wimplicit-fallthrough
  -Wunreachable-code
  -Wmissing-field-initializers
  -Wundef

  -Wno-deprecated-declarations
)

add_executable(chess
  src/generator.cpp
  src/executer.cpp
  src/chess.cpp
)

target_include_directories(chess PRIVATE src)

# target_sources(chess PRIVATE FILE_SET CXX_MODULES FILES
#   src/types/file.cppm
#   # src/hashes.cppm
# )
