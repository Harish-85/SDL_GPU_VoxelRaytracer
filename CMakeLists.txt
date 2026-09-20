cmake_minimum_required(VERSION 4.1)
project(voxel_raytracer)

set(CMAKE_CXX_STANDARD 26)

find_package(SDL3 REQUIRED)
find_package(SDL3_shadercross REQUIRED)

add_executable(voxel_raytracer main.cpp
        VoxelRaytracer.cpp
        VoxelRaytracer.h
        VoxelData.cpp
        VoxelData.h
        VoxelWorld.cpp
        VoxelWorld.h
        SDLFullScreenComputeRenderer.cpp
        SDLFullScreenComputeRenderer.h)

target_link_libraries(voxel_raytracer PRIVATE SDL3::SDL3 SDL3_shadercross::SDL3_shadercross)