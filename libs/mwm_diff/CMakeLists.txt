project(mwm_diff)

set(SRC
  diff.cpp
  diff.hpp
)

omim_add_library(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    base
    coding
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    bsdiff
)


