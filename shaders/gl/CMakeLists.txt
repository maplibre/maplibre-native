file(READ "manifest.json" MANIFEST_JSON_CONTENT)

string(JSON LENGTH LENGTH ${MANIFEST_JSON_CONTENT})
# need to do this because foreach() is inclusive
math(EXPR LENGTH_MIN_ONE "${LENGTH} -1")

set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/include/mbgl/shaders/gl")

# Create list OPENGL_SHADERS_HEADERS with source files
foreach(INDEX RANGE ${LENGTH_MIN_ONE})
  string(JSON ELEM GET ${MANIFEST_JSON_CONTENT} ${INDEX})
  string(JSON HEADER GET ${ELEM} "header")
  list(APPEND OPENGL_SHADERS_HEADERS "${OUTPUT_DIR}/${HEADER}.hpp")
endforeach()
set(OPENGL_SHADERS_HEADERS ${OPENGL_SHADERS_HEADERS} PARENT_SCOPE)

# Generate OpenGL shader headers
add_custom_command(
  OUTPUT ${OPENGL_SHADERS_HEADERS}
  COMMAND node ${CMAKE_CURRENT_SOURCE_DIR}/generate_shader_code.js --input ${CMAKE_CURRENT_SOURCE_DIR} --output ${OUTPUT_DIR}
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/generate_shader_code.js
)
add_custom_target(
  opengl-shader-headers
  DEPENDS ${OPENGL_SHADERS_HEADERS}
)

target_include_directories(
    mbgl-core
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include
)