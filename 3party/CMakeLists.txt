# Fixes CMake deprecation warning:
# Compatibility with CMake < 3.5 will be removed from a future version of CMake.
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)


add_subdirectory(agg)
add_subdirectory(bsdiff-courgette)
add_subdirectory(minizip)
add_subdirectory(open-location-code)
add_subdirectory(opening_hours)
add_subdirectory(stb_image)
add_subdirectory(succinct)
add_subdirectory(monocypher)

if (VULKAN)
  add_subdirectory(vulkan_wrapper)
endif()
