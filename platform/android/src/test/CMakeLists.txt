if(NOT MLN_WITH_TESTS)
    return()
endif()

# The test, benchmark and render test runners, as shared libraries for the
# Android test apps.

add_library(
    mbgl-test-runner SHARED
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    test_runner.cpp
    test_runner_common.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mln/text/local_glyph_rasterizer.cpp
    collator_test_stub.cpp
    number_format_test_stub.cpp
)

target_include_directories(
    mbgl-test-runner
    PRIVATE ${ANDROID_NDK}/sources/android/native_app_glue
    ${PROJECT_SOURCE_DIR}/platform/android/src
    ${PROJECT_SOURCE_DIR}/src
)

find_package(curl CONFIG)

target_link_libraries(
    mbgl-test-runner
    PRIVATE
        MapLibreNative::Base::jni.hpp
        mbgl-compiler-options
        $<$<BOOL:${curl_FOUND}>:curl::curl_static>
        $<LINK_LIBRARY:WHOLE_ARCHIVE,mbgl-test>
)

target_sources(
    mbgl-test-runner
    PRIVATE ${PROJECT_SOURCE_DIR}/platform/default/src/mln/storage/http_file_source.cpp
)

add_custom_command(
    TARGET mbgl-test-runner PRE_BUILD
    COMMAND
        ${CMAKE_COMMAND}
        -E
        make_directory
        ${PROJECT_SOURCE_DIR}/test/results
    COMMAND
        ${CMAKE_COMMAND}
        -E
        tar
        "chf"
        "test/android/app/src/main/assets/data.zip"
        --format=zip
        --files-from=test/android/app/src/main/assets/to_zip.txt
    COMMAND
        ${CMAKE_COMMAND}
        -E
        remove_directory
        ${PROJECT_SOURCE_DIR}/test/results
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_library(
    mbgl-benchmark-runner SHARED
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    benchmark_runner.cpp
    test_runner_common.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mln/text/local_glyph_rasterizer.cpp
    collator_test_stub.cpp
    number_format_test_stub.cpp
    ${PROJECT_SOURCE_DIR}/platform/android/MapLibreAndroid/src/cpp/http_file_source.cpp
)

target_include_directories(
    mbgl-benchmark-runner
    PRIVATE ${ANDROID_NDK}/sources/android/native_app_glue ${PROJECT_SOURCE_DIR}/platform/android/src ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(
    mbgl-benchmark-runner
    PRIVATE
        MapLibreNative::Base::jni.hpp
        mbgl-compiler-options
        $<LINK_LIBRARY:WHOLE_ARCHIVE,mbgl-benchmark>
)

add_custom_command(
    TARGET mbgl-benchmark-runner PRE_BUILD
    COMMAND
        ${CMAKE_COMMAND}
        -E
        make_directory
        ${PROJECT_SOURCE_DIR}/benchmark/results
    COMMAND
        ${CMAKE_COMMAND}
        -E
        tar
        "chf"
        "benchmark/android/app/src/main/assets/data.zip"
        --format=zip
        --files-from=benchmark/android/app/src/main/assets/to_zip.txt
    COMMAND
        ${CMAKE_COMMAND}
        -E
        remove_directory
        ${PROJECT_SOURCE_DIR}/benchmark/results
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_library(
    mbgl-render-test-runner SHARED
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    render_test_runner.cpp
    test_runner_common.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mln/text/local_glyph_rasterizer.cpp
    collator_test_stub.cpp
    number_format_test_stub.cpp
    ${PROJECT_SOURCE_DIR}/platform/android/MapLibreAndroid/src/cpp/http_file_source.cpp
)

if(MLN_WITH_VULKAN)
    target_compile_definitions(mbgl-render-test-runner PRIVATE "MLN_RENDER_BACKEND_VULKAN=1")
endif()

target_include_directories(
    mbgl-render-test-runner
    PRIVATE ${ANDROID_NDK}/sources/android/native_app_glue ${PROJECT_SOURCE_DIR}/platform/android/src ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(
    mbgl-render-test-runner
    PRIVATE
        MapLibreNative::Base::jni.hpp
        android
        log
        mbgl-compiler-options
        mbgl-render-test
)

add_custom_command(
    TARGET mbgl-render-test-runner PRE_BUILD
    COMMAND
        ${CMAKE_COMMAND}
        -E
        tar
        "chf"
        "render-test/android/app/src/main/assets/data.zip"
        --format=zip
        --files-from=render-test/android/app/src/main/assets/to_zip.txt
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

install(TARGETS mbgl-render-test-runner LIBRARY DESTINATION lib)
