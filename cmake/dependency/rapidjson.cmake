# RapidJSON, as mbgl-rapidjson: an installed one, unless MLN_USE_BUILTIN_RAPIDJSON
# is set, and the vendored copy otherwise. Any installed one will do, as there
# has been no release since 1.1.0, so every one says it is that.

if(TARGET mbgl-rapidjson)
    return()
endif()

if(NOT MLN_USE_BUILTIN_RAPIDJSON)
    find_package(RapidJSON CONFIG QUIET)
endif()

if(RapidJSON_FOUND AND TARGET RapidJSON)
    message(STATUS "Using installed RapidJSON ${RapidJSON_VERSION}.")
    # With the definitions the vendored copy has.
    add_library(mbgl-rapidjson INTERFACE IMPORTED)
    target_link_libraries(mbgl-rapidjson INTERFACE RapidJSON)
    target_compile_definitions(mbgl-rapidjson INTERFACE RAPIDJSON_HAS_STDSTRING=1)
    if(WIN32)
        target_compile_definitions(mbgl-rapidjson INTERFACE RAPIDJSON_HAS_CXX11_RVALUE_REFS)
    endif()
else()
    set(MLN_USE_BUILTIN_RAPIDJSON TRUE)
    include(${PROJECT_SOURCE_DIR}/vendor/rapidjson.cmake)
    add_library(mbgl-rapidjson ALIAS mbgl-vendor-rapidjson)
endif()
