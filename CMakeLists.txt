cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(OpenGLVoxels VERSION 1.0.0 LANGUAGES CXX)
find_package(GLEW REQUIRED)

include(FetchContent)
FetchContent_Declare(
  soil
  GIT_REPOSITORY https://github.com/kbranigan/Simple-OpenGL-Image-Library
)
FetchContent_MakeAvailable(soil)

# Create app
add_executable(voxels
  src/main.cpp
)

target_link_libraries(voxels
  PRIVATE GLEW::glew_s
)

target_include_directories(voxels
  PUBLIC include
  PRIVATE ${GLEW_INCLUDE_DIRS}
  PRIVATE ${SOIL_INCLUDE_DIRS}
)
