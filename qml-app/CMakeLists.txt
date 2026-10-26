project(comaps)

find_package(Qt6 REQUIRED COMPONENTS Qml Quick QuickControls2 QuickWidgets Widgets)
# Maemo widgets (value buttons with pickers in the settings); without them the settings use combo boxes.
find_package(qt6maemo QUIET)

qt_policy(SET QTP0001 NEW)

set(SRC
  bridge/bookmark_model.cpp
  bridge/bookmark_model.hpp
  bridge/country_tree_model.cpp
  bridge/country_tree_model.hpp
  bridge/format.cpp
  bridge/format.hpp
  bridge/location.cpp
  bridge/location.hpp
  bridge/map_controller.cpp
  bridge/map_controller.hpp
  bridge/map_search.cpp
  bridge/map_search.hpp
  bridge/place_page.cpp
  bridge/place_page.hpp
  bridge/routing.cpp
  bridge/routing.hpp
  bridge/search_model.cpp
  bridge/search_model.hpp
  bridge/settings.cpp
  bridge/settings.hpp
  main.cpp
  map/map_engine.cpp
  map/map_engine.hpp
  map/map_view.cpp
  map/map_view.hpp
  window/about_widget.cpp
  window/about_widget.hpp
  window/bookmarks_window.cpp
  window/bookmarks_window.hpp
  window/downloader_window.cpp
  window/downloader_window.hpp
  window/main_window.cpp
  window/main_window.hpp
  window/page_window.cpp
  window/page_window.hpp
  window/place_details_window.cpp
  window/place_details_window.hpp
  window/search_window.cpp
  window/search_window.hpp
  window/settings_widget.cpp
  window/settings_widget.hpp
  window/window_manager.cpp
  window/window_manager.hpp
)

file(GLOB QML_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/qml/*.qml")

foreach(qml_file IN LISTS QML_FILES)
  get_filename_component(qml_name "${qml_file}" NAME)
  set_source_files_properties("${qml_file}" PROPERTIES QT_RESOURCE_ALIAS "${qml_name}")
endforeach()

set_source_files_properties(
  qml/AppShell.qml
  qml/Icons.qml
  qml/Metrics.qml
  qml/Style.qml
  PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_executable(${PROJECT_NAME} ${SRC})
# Run from the build tree, the app reads the generated data next to it (runtime_data).
add_dependencies(${PROJECT_NAME} generate_data)
set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC ON)

qt_add_qml_module(${PROJECT_NAME}
  URI Maps
  VERSION 1.0
  OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/qml/Maps
  QML_FILES ${QML_FILES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/bridge
  ${CMAKE_CURRENT_SOURCE_DIR}/map
  ${CMAKE_CURRENT_SOURCE_DIR}/window
)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${3PARTY_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    base
    coding
    drape_frontend
    editor
    geometry
    i18n
    indexer
    kml
    opening_hours
    location_service
    map
    platform
    qt_common
    routing
    search
    storage
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::QuickWidgets
    Qt6::Widgets
)

if (qt6maemo_FOUND)
  message(STATUS "Maemo widgets: ON")
  target_link_libraries(${PROJECT_NAME} PRIVATE qt6maemo::qt6maemo)
  target_compile_definitions(${PROJECT_NAME} PRIVATE MAEMO=1)
else()
  message(STATUS "Maemo widgets: off (qt6-maemo not found)")
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${OMIM_ROOT}/share/applications/comaps.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
if (qt6maemo_FOUND)
  # Hildon's app menu lists the entries below applications/hildon
  install(FILES ${OMIM_ROOT}/share/applications/comaps.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications/hildon)
endif()
install(FILES ${OMIM_ROOT}/qt/res/logo.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps RENAME comaps.png)
