# Linux is the only supported platform.
if (NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif ()

set(PLATFORM_LINUX TRUE)
set(PLATFORM_DESKTOP TRUE)
set(PLATFORM_MAC FALSE)
set(PLATFORM_WIN FALSE)
set(PLATFORM_ANDROID FALSE)
set(PLATFORM_IPHONE FALSE)
