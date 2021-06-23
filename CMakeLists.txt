project(CG-Spring2021)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

set(CMAKE_C_STANDARD   99)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_C_STANDARD_REQUIRED   ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)


if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW) # since 3.12
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
include(cmake/config.cmake)


set(CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo")
if (WIN32)
  if (NOT OSPRAY_DEFAULT_CMAKE_CONFIGURATION_TYPES_SET)
    set(CMAKE_CONFIGURATION_TYPES "${CONFIGURATION_TYPES}"
        CACHE STRING "List of generated configurations." FORCE)
    set(OSPRAY_DEFAULT_CMAKE_CONFIGURATION_TYPES_SET ON
        CACHE INTERNAL "Default CMake configuration types set.")
  endif()
else()
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type." FORCE)
  endif()
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CONFIGURATION_TYPES})
endif()



set(OpenGL_GL_PREFERENCE "LEGACY")
find_package(OpenGL 2 REQUIRED)
find_package(glfw3 REQUIRED)
find_package(embree 3.1 REQUIRED)
find_package(ospcommon REQUIRED)
find_package(openvkl REQUIRED)
find_package(ospray REQUIRED)
find_package(imgui REQUIRED)

include(${OSPRAY_SDK_USE_FILE})
ospcommon_configure_build_type()

add_subdirectory(src)