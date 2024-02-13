cmake_minimum_required(VERSION 3.26.0)
project(MetropoliaChess VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SRC ${PROJECT_SOURCE_DIR} src/*.cpp libs/tinygltf/*.cc libs/imgui/*.cpp)
add_executable(MetropoliaChess ${SRC})
target_include_directories(MetropoliaChess PUBLIC src/ libs/imgui/ libs/tinygltf/ libs/glm)

#glfw
add_subdirectory(libs/glfw EXCLUDE_FROM_ALL)
#glfw3webgpu
add_subdirectory(libs/glfw3webgpu EXCLUDE_FROM_ALL)
#webgpu
add_subdirectory(libs/webgpu EXCLUDE_FROM_ALL)
target_link_libraries(MetropoliaChess PRIVATE glfw webgpu glfw3webgpu)

target_copy_webgpu_binaries(MetropoliaChess)

file(COPY ${PROJECT_SOURCE_DIR}/assets DESTINATION ${PROJECT_BINARY_DIR})