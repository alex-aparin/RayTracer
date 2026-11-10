cmake_minimum_required(VERSION 3.10)
project(ray_tracer C)

set(CMAKE_C_STANDARD 99)

# Core: math, intersections, lights, recursive tracing. Knows nothing about scenes.
add_library(ray_tracer STATIC
    src/ray_tracer.c
    src/objects/sphere.c
    src/objects/plane.c
)
target_include_directories(ray_tracer PUBLIC include)

if(UNIX)
    target_link_libraries(ray_tracer PUBLIC m)
endif()

# Demo scene: gradient sphere, checkered plane, polygon mountains.
add_library(demo_scene STATIC scenes/demo.c)
target_include_directories(demo_scene PUBLIC scenes)
target_link_libraries(demo_scene PUBLIC ray_tracer)

# Chessboard scene: mirror balls on a checkerboard floor.
add_library(chess_balls_scene STATIC scenes/chess_balls.c)
target_include_directories(chess_balls_scene PUBLIC scenes)
target_link_libraries(chess_balls_scene PUBLIC ray_tracer)

add_executable(ppm_file_generator tools/ppm_file_generator/main.c)
target_link_libraries(ppm_file_generator PRIVATE demo_scene chess_balls_scene)

# Tests
enable_testing()
add_executable(smoke_test tests/smoke_test.c)
target_link_libraries(smoke_test PRIVATE demo_scene chess_balls_scene)
add_test(NAME smoke_test COMMAND smoke_test)
