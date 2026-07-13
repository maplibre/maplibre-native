if(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
    set(_wgpu_lib_arch "aarch64-linux-android")
elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
    set(_wgpu_lib_arch "armv7-linux-androideabi")
elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
    set(_wgpu_lib_arch "x86_64-linux-android")
elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86")
    set(_wgpu_lib_arch "i686-linux-android")
endif()
set(_wgpu_lib_name "libwgpu_native.a")



macro(mln_wgpu_android_link_library target)
    find_package(Threads REQUIRED)
    target_link_libraries(${target} INTERFACE
        Threads::Threads
        ${CMAKE_DL_LIBS}
        log
        android
    )
endmacro()
