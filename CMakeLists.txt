cmake_minimum_required(VERSION 3.10)

project(GLTP VERSION 0.1 LANGUAGES CXX)

# find_package(assimp REQUIRED)

set(SRC src/program.cc src/utils.cc src/texture.cc src/mesh.cc src/shadow_map.cc src/camera.cc)

add_executable(GLTPexe src/main.cc ${SRC})
target_compile_features(GLTPexe PUBLIC cxx_std_17)
target_compile_options(GLTPexe PUBLIC -Wall -Wextra -pedantic -march=native)
target_include_directories(GLTPexe PUBLIC
                          "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(GLTPexe -lGLEW -lGL -lglut -lassimp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

