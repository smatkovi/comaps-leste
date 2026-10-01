cmake_minimum_required(VERSION 3.22.1)

project(omim C CXX)

# The internal libraries depend on each other cyclically, so they are always static
# (a packaging -DBUILD_SHARED_LIBS=ON must not change that).
set(BUILD_SHARED_LIBS OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
# Our code does not rely on gnu extensions.
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Disable build-id generation to make builds reproducible.
if (NOT APPLE)
  add_link_options("LINKER:--build-id=none")
endif()

# Fixes warning ld: warning: ignoring duplicate libraries on Mac and Windows.
if (POLICY CMP0156)
  cmake_policy(SET CMP0156 NEW)
endif()

set(OMIM_ROOT ${CMAKE_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${OMIM_ROOT}/cmake")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

include(OmimPlatform)
include(OmimOptions)
include(OmimConfig)
include(OmimHelpers)

message(STATUS "Using compiler ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

if (CMAKE_UNITY_BUILD)
  message(STATUS "Using Unity Build with batch ${CMAKE_UNITY_BUILD_BATCH_SIZE}, use -DCMAKE_UNITY_BUILD=OFF to disable it.")
endif()

if (USE_CCACHE)
  find_program(CCACHE_PROGRAM ccache HINTS /usr/local/bin/)
  if (CCACHE_PROGRAM)
    message(STATUS "Using ccache, use -DUSE_CCACHE=OFF to disable it.")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
  endif()
endif()

# Global compile options for all configurations.
add_compile_options(-ffast-math $<$<CXX_COMPILER_ID:GNU>:-Wno-psabi>)

# Built-in CMake configurations: Debug, Release, RelWithDebInfo, MinSizeRel
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  add_definitions(-DDEBUG)
  add_compile_options(-fno-omit-frame-pointer)
elseif (${CMAKE_BUILD_TYPE} MATCHES "Rel")
  add_definitions(-DRELEASE)
  add_compile_options(-O3)
else()
  message(FATAL_ERROR "Unknown build type: " ${CMAKE_BUILD_TYPE})
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
  add_compile_options(-fno-omit-frame-pointer)
endif()

message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

find_package(Threads REQUIRED)

# Scripts

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Network Widgets Gui Xml Svg OpenGL OpenGLWidgets)

# To allow #include "base/file_name.hpp" in all sources.
include_directories("${CMAKE_HOME_DIRECTORY}" "${CMAKE_HOME_DIRECTORY}/libs" "${CMAKE_HOME_DIRECTORY}/tools")

if (USE_PCH)
  message(STATUS "Precompiled headers are ON")
  set(OMIM_PCH_TARGET_NAME "omim_pch")
  add_precompiled_headers(
    ${OMIM_ROOT}/precompiled_headers.hpp
    ${OMIM_PCH_TARGET_NAME}
  )
endif()

find_package(Freetype REQUIRED)
find_package(harfbuzz CONFIG REQUIRED)
find_package(expat CONFIG REQUIRED)
find_package(jansson CONFIG QUIET)
if (NOT jansson_FOUND)
  # Debian's libjansson-dev only ships a pkg-config file.
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(JANSSON REQUIRED IMPORTED_TARGET GLOBAL jansson)
  add_library(jansson::jansson ALIAS PkgConfig::JANSSON)
endif()
find_package(pugixml CONFIG REQUIRED)
find_package(utf8cpp CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
if (NOT TARGET glm::glm-header-only)
  # Debian's glm only exports glm::glm, which is header-only there.
  add_library(glm::glm-header-only ALIAS glm::glm)
endif()
find_package(ZLIB REQUIRED)
find_package(Protobuf CONFIG QUIET)
if (NOT Protobuf_FOUND)
  # Debian's libprotobuf-dev has no CMake package; CMake's FindProtobuf provides the same targets.
  find_package(Protobuf REQUIRED)
endif()

# Boost is used header-only; take it from the system.
find_package(Boost 1.85 CONFIG REQUIRED)
set(3PARTY_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})

# Include 3party dependencies.
add_subdirectory(3party)

# Fix for #include <boost/regex.hpp>
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

# Used in qt/ and shaders/
find_package(Python3 REQUIRED COMPONENTS Interpreter)

include(GNUInstallDirs)

add_subdirectory(libs)
add_subdirectory(tools/skin_generator)
add_subdirectory(data)
add_subdirectory(qt/qt_common)
add_subdirectory(qml-app)
