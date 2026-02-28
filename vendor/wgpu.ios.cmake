if(CMAKE_OSX_SYSROOT MATCHES "[Ss]imulator")
    set(_wgpu_lib_arch "aarch64-apple-ios-sim")
else()
    set(_wgpu_lib_arch "aarch64-apple-ios")
endif()
set(_wgpu_cargo_features "--no-default-features;--features;metal,wgsl,spirv,glsl")

macro(mln_wgpu_ios_append_search_paths paths)
    # With the Xcode generator, CMAKE_OSX_SYSROOT is "iphoneos" at configure time even when
    # the user builds for simulator. Search both iOS architectures so the right one is found.
    if(_wgpu_lib_arch STREQUAL "aarch64-apple-ios")
        list(APPEND ${paths} "${_mln_wgpu_source_dir}/target/aarch64-apple-ios-sim/release")
    else()
        list(APPEND ${paths} "${_mln_wgpu_source_dir}/target/aarch64-apple-ios/release")
    endif()
endmacro()

macro(mln_wgpu_ios_find_library)
    foreach(_search_path ${_wgpu_lib_search_paths})
        if(EXISTS "${_search_path}/${_wgpu_lib_name}")
            set(WGPU_LIBRARY "${_search_path}/${_wgpu_lib_name}" CACHE FILEPATH "wgpu-native library")
            break()
        endif()
    endforeach()
endmacro()

macro(mln_wgpu_ios_setup_cargo_env env_prefix_var)
    # On iOS, bindgen's clang_macro_fallback() uses a separate clang process that
    # doesn't inherit the .clang_arg() settings from build.rs. We must set
    # BINDGEN_EXTRA_CLANG_ARGS so it can find system headers (needed for UINT32_MAX etc.)
    execute_process(
        COMMAND xcrun --sdk iphoneos --show-sdk-path
        OUTPUT_VARIABLE _ios_sdk_path
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(${env_prefix_var} ${CMAKE_COMMAND} -E env "BINDGEN_EXTRA_CLANG_ARGS=--target=arm64-apple-ios -isysroot ${_ios_sdk_path}")
endmacro()
