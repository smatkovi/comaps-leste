project(vulkan_wrapper)

set(SRC
  vulkan_wrapper.cpp
  vulkan_wrapper.h
)

add_library(${PROJECT_NAME} ${SRC})

find_package(VulkanHeaders CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Headers)

target_include_directories(${PROJECT_NAME} PUBLIC .)

# libvulkan is dlopen()ed at runtime if present.
target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_DL_LIBS})
