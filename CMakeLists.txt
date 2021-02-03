# CMakeList.txt : projet CMake pour raymarcher, incluez la source et définissez
# la logique spécifique au projet ici.
#
cmake_minimum_required (VERSION 3.8)

project(raytracer VERSION 1.0 LANGUAGES CXX)

add_subdirectory("CxxSwizzle")

# Ajoutez une source à l'exécutable de ce projet.
add_executable (raytracer "raytracer.cpp" "raytracer.h")

target_include_directories(raytracer PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/CxxSwizzle/include"
                          )