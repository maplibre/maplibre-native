cmake_minimum_required(VERSION 2.8.12)
project(harfbuzzFreetype)

add_subdirectory(freetype)
add_subdirectory(harfbuzz)


target_compile_definitions(harfbuzz PRIVATE -DHAVE_FREETYPE)


