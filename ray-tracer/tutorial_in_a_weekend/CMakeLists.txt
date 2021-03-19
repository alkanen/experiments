cmake_minimum_required(VERSION 3.11)

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

if(LINUX)
  set(CMAKE_CXX_FLAGS "-O3 -Wall")
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.7.3
)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

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
  src/onb.hpp
  src/pdf.hpp
)

# Link against the dependency of Intel TBB (for parallel C++17 algorithms)
if(LINUX)
  target_link_libraries(raytracer tbb)
endif()
target_link_libraries(raytracer nlohmann_json::nlohmann_json)
