cmake_minimum_required(VERSION 2.8.12)
project(harfbuzzFreetype)

option(BAZEL_BUILD "Bazel build option" OFF)

if(BAZEL_BUILD) 

    set (project_headers harfbuzz/src/hb-ft.h)
endif()

add_subdirectory(freetype)
add_subdirectory(harfbuzz)

target_compile_definitions(harfbuzz PRIVATE -DHAVE_FREETYPE)



if(BAZEL_BUILD) 

    message("BAZEL_BUILD patch")

    install(FILES harfbuzz/src/hb-ft.h DESTINATION include)
    install(FILES ${project_headers} DESTINATION include)
endif()