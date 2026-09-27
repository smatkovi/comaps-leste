project(location_service)

find_package(Qt6 REQUIRED COMPONENTS Positioning)

set(SRC
  location_service.cpp
  location_service.hpp
  qt_location_service.hpp
  qt_location_service.cpp
)

omim_add_library(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    platform
    Qt6::Positioning
  PRIVATE
    base
)

target_compile_definitions(${PROJECT_NAME} PRIVATE "QT_LOCATION_SERVICE")
set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC ON)
