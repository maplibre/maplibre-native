# Include guard
if(TARGET rustutils)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    Corrosion
    GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
    GIT_TAG v0.5.1 # Optionally specify a commit hash, version tag or branch here
)
FetchContent_MakeAvailable(Corrosion)

corrosion_import_crate(MANIFEST_PATH ${CMAKE_CURRENT_LIST_DIR}/Cargo.toml)

# Define output directories for generated bindings
set(RUSTUTILS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/rustutils_bindings")
set(RUSTUTILS_INCLUDE_DIR "${RUSTUTILS_OUTPUT_DIR}/cpp/include")
set(RUSTUTILS_SRC_DIR "${RUSTUTILS_OUTPUT_DIR}/cpp/src")

# Ensure the output directories exist
file(MAKE_DIRECTORY ${RUSTUTILS_INCLUDE_DIR})
file(MAKE_DIRECTORY ${RUSTUTILS_SRC_DIR})

set(RUSTUTILS_RS_FILES
    ${CMAKE_CURRENT_LIST_DIR}/src/color.rs
)

# Initialize variables for generated files
set(RUSTUTILS_GENERATED_SOURCES)
set(RUSTUTILS_GENERATED_HEADERS)

# Transform .rs to .rs.cpp and .hpp
foreach(rs_file ${RUSTUTILS_RS_FILES})
    get_filename_component(base_name ${rs_file} NAME_WE)
    list(APPEND RUSTUTILS_GENERATED_SOURCES "${RUSTUTILS_SRC_DIR}/${base_name}.rs.cpp")
    list(APPEND RUSTUTILS_GENERATED_HEADERS "${RUSTUTILS_INCLUDE_DIR}/rustutils/${base_name}.hpp")
endforeach()

add_custom_command(
    OUTPUT ${RUSTUTILS_GENERATED_SOURCES} ${RUSTUTILS_GENERATED_HEADERS}
    COMMAND ${CMAKE_COMMAND} -E env
        ${CMAKE_CURRENT_LIST_DIR}/generate.sh ${RUSTUTILS_OUTPUT_DIR} ${RUSTUTILS_RS_FILES}
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/generate.sh ${RUSTUTILS_RS_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    COMMENT "Generating C++ bindings for Rust sources"
    VERBATIM
)

add_custom_target(rustutils_bindings DEPENDS ${RUSTUTILS_GENERATED_SOURCES} ${RUSTUTILS_GENERATED_HEADERS})

add_library(mbgl-rustutils STATIC
    ${RUSTUTILS_GENERATED_SOURCES}
)

add_dependencies(mbgl-rustutils rustutils_bindings)

target_include_directories(mbgl-rustutils PUBLIC
    ${RUSTUTILS_INCLUDE_DIR}
)

target_link_libraries(mbgl-rustutils PUBLIC rustutils)
