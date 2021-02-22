cmake_minimum_required(VERSION 3.5)

project(RayTracerTutorial)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Link against the dependency of Intel TBB (for parallel C++17 algorithms)
# target_link_libraries(raytracer tbb)

add_executable(raytracer
  src/main.cpp
  src/camera.hpp
  src/color.hpp
  src/hittable.hpp
  src/hittable_list.hpp
  src/material.hpp
  src/ray.hpp
  src/rtweekend.hpp
  src/sphere.hpp
  src/vec3.hpp
  src/stb_image.h
  src/stb_image_write.h
)


add_executable(raytracer_scene
  src/main_scene.cpp
  src/camera.hpp
  src/color.hpp
  src/hittable.hpp
  src/hittable_list.hpp
  src/material.hpp
  src/ray.hpp
  src/rtweekend.hpp
  src/sphere.hpp
  src/vec3.hpp
  src/image_plane.hpp
  src/greenmeadow.h
  src/greenmeadow.cpp
  src/stb_image.h
  src/stb_image_write.h
)
