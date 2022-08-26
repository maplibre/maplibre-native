add_executable(map-renderer src/main.cpp src/maprenderer.cpp src/spng.c)

target_include_directories(map-renderer PUBLIC include)
target_include_directories(map-renderer PRIVATE vendor)

target_link_libraries(map-renderer PRIVATE Mapbox::Base mbgl-core)

install(TARGETS map-renderer DESTINATION "${CMAKE_INSTALL_BINDIR}")
