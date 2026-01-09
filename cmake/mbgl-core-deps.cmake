# this file defines a target that will generate a mbgl-core-deps.txt which includes the linker
# flags that would need to be passed to an executable or library that links with
# the static mbgl-core library

set(DUMMY_FILE "${CMAKE_BINARY_DIR}/dummy.cpp")
add_custom_command(
    OUTPUT ${DUMMY_FILE}
    COMMAND ${CMAKE_COMMAND} -E echo "int main() {}" > ${DUMMY_FILE}
    COMMENT "Generating dummy.cpp"
    VERBATIM
)

# https://stackoverflow.com/questions/34165365/retrieve-all-link-flags-in-cmake
set(CMAKE_ECHO_STANDARD_LIBRARIES ${CMAKE_CXX_STANDARD_LIBRARIES})
set(CMAKE_ECHO_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_ECHO_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS})
set(CMAKE_ECHO_IMPLICIT_LINK_DIRECTORIES ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES})
set(
    CMAKE_ECHO_LINK_EXECUTABLE
    "<CMAKE_COMMAND> -E echo \"<FLAGS> <LINK_FLAGS> <LINK_LIBRARIES>\" > <TARGET>"
)

add_executable(mbgl-core-deps EXCLUDE_FROM_ALL "${DUMMY_FILE}")
target_link_libraries(mbgl-core-deps mbgl-core)
add_custom_target(generate_dummy DEPENDS ${DUMMY_FILE})
add_dependencies(mbgl-core-deps generate_dummy)

set_target_properties(
    mbgl-core-deps
        PROPERTIES
            LINKER_LANGUAGE ECHO
            SUFFIX          ".txt"
)
