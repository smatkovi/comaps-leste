# Runtime data: generated into ${CMAKE_BINARY_DIR}/runtime_data and installed to share/comaps/data,
# where platform_linux.cpp looks for it (../share/comaps/data next to bin/).

set(GENERATED_DATA_DIR ${CMAKE_BINARY_DIR}/runtime_data)
set(GENERATED_DATA_STAMP ${CMAKE_BINARY_DIR}/data.stamp)

add_custom_command(
  OUTPUT ${GENERATED_DATA_STAMP}
  COMMAND ${OMIM_ROOT}/tools/generate_data.sh ${OMIM_ROOT} ${GENERATED_DATA_DIR}
          $<TARGET_FILE:skin_generator_tool> $<TARGET_FILE:protobuf::protoc>
  COMMAND ${CMAKE_COMMAND} -E touch ${GENERATED_DATA_STAMP}
  DEPENDS skin_generator_tool ${OMIM_ROOT}/tools/generate_data.sh
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Generating runtime data"
  USES_TERMINAL
)
add_custom_target(generate_data ALL DEPENDS ${GENERATED_DATA_STAMP})

# Base world maps shipped with the app; the version must match data/countries.txt.
set(WORLD_MAPS_URL "https://mapgen-fi-1.comaps.app/maps/2026.06.28/260830")
set(WORLD_MAPS_DIR ${CMAKE_BINARY_DIR}/world_maps)
file(DOWNLOAD ${WORLD_MAPS_URL}/World.mwm ${WORLD_MAPS_DIR}/World.mwm
     EXPECTED_HASH SHA256=4dc68624af3102471e03bc05b02ad8b7a7ae3a7b6cd30ecb64228b14f9d4df03)
file(DOWNLOAD ${WORLD_MAPS_URL}/WorldCoasts.mwm ${WORLD_MAPS_DIR}/WorldCoasts.mwm
     EXPECTED_HASH SHA256=a4d6f470b19137a624962ada2fe0d9c803e1f8925233abd3ce87a378abe2fe87)

set(DATA_INSTALL_DIR ${CMAKE_INSTALL_DATADIR}/comaps/data)

install(DIRECTORY
  ${GENERATED_DATA_DIR}/countries-strings
  ${GENERATED_DATA_DIR}/fonts
  ${GENERATED_DATA_DIR}/symbols
  DESTINATION ${DATA_INSTALL_DIR}
)

install(FILES
  ${GENERATED_DATA_DIR}/welcome.html
  ${GENERATED_DATA_DIR}/categories.txt
  ${GENERATED_DATA_DIR}/categories_brands.txt
  ${GENERATED_DATA_DIR}/classificator.txt
  ${GENERATED_DATA_DIR}/colors.txt
  ${GENERATED_DATA_DIR}/copyright.html
  ${GENERATED_DATA_DIR}/countries.txt
  ${GENERATED_DATA_DIR}/countries_meta.txt
  ${GENERATED_DATA_DIR}/editor.config
  ${GENERATED_DATA_DIR}/packed_polygons.bin
  ${GENERATED_DATA_DIR}/patterns.txt
  ${GENERATED_DATA_DIR}/subtypes.csv
  ${GENERATED_DATA_DIR}/transit_colors.txt
  ${GENERATED_DATA_DIR}/types.txt
  ${GENERATED_DATA_DIR}/mapcss-mapping.csv
  ${WORLD_MAPS_DIR}/World.mwm
  ${WORLD_MAPS_DIR}/WorldCoasts.mwm
  DESTINATION ${DATA_INSTALL_DIR}
)

foreach(style
    default_light default_dark outdoors_light outdoors_dark
    walking_light walking_dark walking_outdoor_light walking_outdoor_dark
    cycling_light cycling_dark cycling_outdoor_light cycling_outdoor_dark
    driving_light driving_dark driving_outdoor_light driving_outdoor_dark
    public-transport_light public-transport_dark public-transport_outdoor_light public-transport_outdoor_dark
    vehicle_light vehicle_dark)
  install(FILES ${GENERATED_DATA_DIR}/drules_proto_${style}.bin DESTINATION ${DATA_INSTALL_DIR})
endforeach()
