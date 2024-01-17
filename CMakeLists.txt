cmake_minimum_required(VERSION 3.26.0)
project(MetropoliaChess VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SRC ${PROJECT_SOURCE_DIR} src/*.cpp)
add_executable(MetropoliaChess ${SRC})
target_include_directories(MetropoliaChess PUBLIC src/)
