project(desktop)

QT6_ADD_RESOURCES(RES_SOURCES res/resources.qrc)

set(SRC
  about.cpp
  about.hpp
  bookmark_dialog.cpp
  bookmark_dialog.hpp
  create_feature_dialog.cpp
  create_feature_dialog.hpp
  draw_widget.cpp
  draw_widget.hpp
  editor_dialog.cpp
  editor_dialog.hpp
  html_processor.cpp
  html_processor.hpp
  info_dialog.cpp
  info_dialog.hpp
  main.cpp
  mainwindow.cpp
  mainwindow.hpp
  osm_auth_dialog.cpp
  osm_auth_dialog.hpp
  place_page_dialog_common.cpp
  place_page_dialog_common.hpp
  place_page_dialog_developer.cpp
  place_page_dialog_developer.hpp
  place_page_dialog_user.cpp
  place_page_dialog_user.hpp
  preferences_dialog.cpp
  preferences_dialog.hpp
  popup_menu_holder.cpp
  popup_menu_holder.hpp
  routing_settings_dialog.cpp
  routing_settings_dialog.hpp
  routing_turns_visualizer.cpp
  routing_turns_visualizer.hpp
  ruler.cpp
  ruler.hpp
  screenshoter.cpp
  screenshoter.hpp
  search_panel.cpp
  search_panel.hpp
  selection.hpp
  star_rating_widget.cpp
  star_rating_widget.hpp
  update_dialog.cpp
  update_dialog.hpp
)

omim_add_executable(${PROJECT_NAME} ${RES_SOURCES} ${SRC})

set_target_properties(${PROJECT_NAME} PROPERTIES
  AUTOMOC ON
  OUTPUT_NAME comaps
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    base
    drape_frontend
    geometry
    indexer
    kml
    location_service
    map
    platform
    qt_common
    routing
    search
    storage
  PRIVATE
    coding
    editor
    i18n
    gflags::gflags
)

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES ${OMIM_ROOT}/share/applications/comaps.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

install(FILES ${PROJECT_SOURCE_DIR}/res/logo.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps
        RENAME comaps.png)

add_subdirectory(qt_common)
