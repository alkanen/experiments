cmake_minimum_required(VERSION 3.5)

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

set(CMAKE_CXX_COMPILER g++-10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(RayTracerTutorial)

add_executable(raytracer
  src/main.cpp
  src/aabb.hpp
  src/aarect.hpp
  src/box.hpp
  src/bvh.hpp
  src/camera.hpp
  src/color.hpp
  src/constant_medium.hpp
  src/hittable.hpp
  src/hittable_list.hpp
  src/material.hpp
  src/moving_sphere.hpp
  src/perlin.hpp
  src/ray.hpp
  src/rtw_stb_image.hpp
  src/rtweekend.hpp
  src/sphere.hpp
  src/texture.hpp
  src/vec3.hpp
)

# Link against the dependency of Intel TBB (for parallel C++17 algorithms)
if(LINUX)
  target_link_libraries(raytracer tbb)
endif()

