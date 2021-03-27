cmake_minimum_required(VERSION 3.11)

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

if(LINUX)
  #set(CMAKE_CXX_FLAGS "-O0 -Wall -g -DDEBUG")
  set(CMAKE_CXX_FLAGS "-Ofast -g")
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
  src/aabb.cpp
  src/aarect.hpp
  src/aarect.cpp
  src/camera.hpp
  src/camera.cpp
  src/color.hpp
  src/color.cpp
  src/constants.hpp
  src/hittable.hpp
  src/hittable.cpp
  src/material.hpp
  src/material.cpp
  src/perlin.hpp
  src/perlin.cpp
  src/ray.hpp
  src/ray.cpp
  src/rtweekend.hpp
  src/rtweekend.cpp
  src/texture.hpp
  src/texture.cpp
  src/vec3.hpp
  src/vec3.cpp
  src/onb.hpp
  src/onb.cpp
  src/pdf.hpp
  src/pdf.cpp
  src/sphere.hpp
  src/sphere.cpp
  src/box.hpp
  src/box.cpp
  src/hittable_list.hpp
  src/hittable_list.cpp
  src/bvh.hpp
  src/bvh.cpp
  src/moving_sphere.hpp
  src/moving_sphere.cpp
  src/constant_medium.hpp
  src/constant_medium.cpp

  src/stb_image.h
  src/stb_image_write.h
  src/stb_image_impl.cpp
  src/rtw_stb_image.cpp
  src/png.hpp
  src/png.cpp

  src/world.hpp
  src/world.cpp
  src/camera.hpp
  src/camera.cpp
  src/render_params.hpp
  src/render_params.cpp
  src/ray_color.hpp
  src/ray_color.cpp
)

# Link against the dependency of Intel TBB (for parallel C++17 algorithms)
if(LINUX)
  target_link_libraries(raytracer tbb)
endif()
target_link_libraries(raytracer nlohmann_json::nlohmann_json)
