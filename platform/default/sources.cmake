# Groups of the default platform's sources, for the platform files to add to
# `mbgl-core` as they need them.

# Utilities that the core itself uses. Every platform needs them, and none has
# its own yet.
set(
    MLN_DEFAULT_UTIL_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/platform/time.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/util/compression.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/util/filesystem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/util/monotonic_timer.cpp
)

# The default file source stack: the file source manager, which creates all
# of these, and the file sources it creates, including the offline database.
set(
    MLN_DEFAULT_FILE_SOURCE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/storage/asset_file_source.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/storage/database_file_source.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/storage/file_source_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/storage/file_source_request.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/storage/local_file_request.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/storage/local_file_source.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/storage/main_resource_loader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/storage/mbtiles_file_source.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/storage/offline.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/storage/offline_database.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/storage/offline_download.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/storage/online_file_source.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/storage/$<IF:$<BOOL:${MLN_WITH_PMTILES}>,pmtiles_file_source.cpp,pmtiles_file_source_stub.cpp>
)

# Headless rendering, with the headless backend of the renderer built.
set(
    MLN_DEFAULT_HEADLESS_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/gfx/headless_backend.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/gfx/headless_frontend.cpp
    $<$<BOOL:${MLN_WITH_OPENGL}>:${CMAKE_CURRENT_LIST_DIR}/src/mln/gl/headless_backend.cpp>
    $<$<BOOL:${MLN_WITH_METAL}>:${CMAKE_CURRENT_LIST_DIR}/src/mln/mtl/headless_backend.cpp>
    $<$<BOOL:${MLN_WITH_VULKAN}>:${CMAKE_CURRENT_LIST_DIR}/src/mln/vulkan/headless_backend.cpp>
)

# The run loop, async tasks and timers implemented with libuv.
set(
    MLN_DEFAULT_LIBUV_RUN_LOOP_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/util/async_task.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/util/run_loop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/util/timer.cpp
)

# Decoding images, by their format, with libpng, libjpeg and libwebp, for
# the platforms without a decoder of their own.
set(
    MLN_DEFAULT_IMAGE_DECODER_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/util/image.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/util/jpeg_reader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/util/png_reader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/util/webp_reader.cpp
)

# Locale-aware text: collation, number formatting and case mapping, with
# nunicode and ICU.
set(
    MLN_DEFAULT_LOCALE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/i18n/collator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/i18n/number_format.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/util/string_stdlib.cpp
)

# Unicode text: bidirectional text and character properties with ICU, and
# conversions between UTF-8 and UTF-16.
set(
    MLN_DEFAULT_UNICODE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/text/bidi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/util/i18n.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/util/utf.cpp
)
