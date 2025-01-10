# Include guard
if(TARGET rustutils AND MLN_USE_RUST)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    Corrosion
    GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
    GIT_TAG v0.5 # Optionally specify a commit hash, version tag or branch here
)
FetchContent_MakeAvailable(Corrosion)

corrosion_import_crate(MANIFEST_PATH ${CMAKE_CURRENT_LIST_DIR}/Cargo.toml)

set(RUSTUTILS_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/cpp/include")
set(RUSTUTILS_SRC "${CMAKE_CURRENT_LIST_DIR}/cpp/src/lib.rs.cc")

add_library(mbgl-rustutils STATIC
  ${RUSTUTILS_SRC}
)

target_include_directories(mbgl-rustutils PUBLIC
    ${RUSTUTILS_INCLUDE_DIR}
)

target_link_libraries(mbgl-rustutils PUBLIC rustutils)
