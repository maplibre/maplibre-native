if(DEFINED ENV{VCINSTALLDIR})
    set(VCPKG_OVERLAY_TRIPLETS ${CMAKE_CURRENT_LIST_DIR}/vendor/vcpkg-custom-triplets)
    set(VCPKG_DEFAULT_TRIPLET $ENV{VSCMD_ARG_TGT_ARCH}-windows)
    set(VCPKG_TARGET_TRIPLET $ENV{VSCMD_ARG_TGT_ARCH}-windows)

    include(${CMAKE_CURRENT_LIST_DIR}/vendor/vcpkg/scripts/buildsystems/vcpkg.cmake)
endif()
