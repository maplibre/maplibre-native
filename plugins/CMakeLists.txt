enable_testing()

# `mln-plugin-tests` builds the tests of every plugin, which ctest runs from the repository root.
add_custom_target(mln-plugin-tests)

# The manifest of the render tests that are the repository's own, for the renderer of this build, and the name of that
# renderer in the `backends` of a plugin.json.
if(MLN_WITH_METAL)
    set(MLN_PLUGIN_BACKEND metal)
    set(MLN_PLUGIN_MANIFEST metrics/ios-metal-render-test-runner-style.json)
elseif(MLN_WITH_VULKAN)
    set(MLN_PLUGIN_BACKEND vulkan)
    set(MLN_PLUGIN_MANIFEST metrics/linux-vulkan.json)
else()
    set(MLN_PLUGIN_BACKEND opengl)
    set(MLN_PLUGIN_MANIFEST metrics/linux-opengl.json)
endif()

# The plugin in plugins/<id>, built from its directory layout:
#   shared/cpp/*.cpp, shared/include and vendor   the library mln-<id>
#   tests/*.test.cpp                              googletest tests of the library (tests/host: see below)
#   render-tests                                  the plugin's own render tests with render-tests/manifest.json, or else
#                                                 the repository's tests of the layer types that the plugin registers,
#                                                 without the tests in render-tests/ignores.json
# The render tests run when the renderer of this build is one of the `backends`.
function(mln_plugin id backends)
    string(REPLACE "-" "_" snake ${id})
    string(TOUPPER ${snake} upper)
    set(directory ${CMAKE_CURRENT_SOURCE_DIR}/plugins/${id})
    set(relative plugins/plugins/${id})

    file(GLOB sources CONFIGURE_DEPENDS ${directory}/shared/cpp/*.cpp)
    add_library(mln-${id} STATIC ${sources})
    target_compile_features(mln-${id} PUBLIC cxx_std_20)
    set_target_properties(mln-${id} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_include_directories(mln-${id} PUBLIC ${directory}/shared/include)
    if(EXISTS ${directory}/vendor)
        target_include_directories(mln-${id} PRIVATE ${directory}/vendor)
    endif()
    target_link_libraries(mln-${id} PUBLIC mbgl-core)
    if(MLN_WITH_METAL)
        # A plugin may include its Metal shaders only in a Metal build: MLN_<ID>_METAL.
        target_compile_definitions(mln-${id} PRIVATE MLN_${upper}_METAL=1)
    endif()
    set(MLN_PLUGIN_LIBRARIES ${MLN_PLUGIN_LIBRARIES} mln-${id} PARENT_SCOPE)

    file(GLOB tests CONFIGURE_DEPENDS ${directory}/tests/*.test.cpp)
    if(tests)
        add_executable(mln-${id}-tests EXCLUDE_FROM_ALL ${tests} ${PROJECT_SOURCE_DIR}/vendor/googletest/googletest/src/gtest_main.cc)
        target_link_libraries(mln-${id}-tests PRIVATE mln-${id} mbgl-vendor-googletest)
        add_dependencies(mln-plugin-tests mln-${id}-tests)
        add_test(NAME mln-${id}-tests COMMAND mln-${id}-tests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    endif()

    if(MLN_PLUGIN_BACKEND IN_LIST backends)
        add_executable(mln-${id}-render-test EXCLUDE_FROM_ALL ${CMAKE_CURRENT_SOURCE_DIR}/render_test.cpp)
        target_compile_definitions(mln-${id}-render-test PRIVATE
            MLN_PLUGIN_HEADER="${snake}.hpp" MLN_PLUGIN_REGISTER=mln_${snake}_register)
        target_link_libraries(mln-${id}-render-test PRIVATE mln-${id} mbgl-render-test)
        add_dependencies(mln-plugin-tests mln-${id}-render-test)

        if(EXISTS ${directory}/render-tests/manifest.json)
            set(options --manifestPath ${relative}/render-tests/manifest.json)
        else()
            set(options --manifestPath ${MLN_PLUGIN_MANIFEST} --plugin-layers)
            if(EXISTS ${directory}/render-tests/ignores.json)
                list(APPEND options --ignore ${relative}/render-tests/ignores.json)
            endif()
        endif()
        add_test(NAME mln-${id}-render-test COMMAND mln-${id}-render-test ${options} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    endif()
endfunction()

# Every directory of plugins/plugins that has a plugin.json. Its `backends` default to all.
file(GLOB manifests CONFIGURE_DEPENDS plugins/*/plugin.json)
foreach(manifest ${manifests})
    get_filename_component(directory ${manifest} DIRECTORY)
    get_filename_component(id ${directory} NAME)
    file(READ ${manifest} json)
    string(JSON backends_json ERROR_VARIABLE no_backends GET "${json}" backends)
    if(no_backends)
        set(backends opengl vulkan metal)
    else()
        set(backends)
        string(JSON count LENGTH "${backends_json}")
        math(EXPR last "${count} - 1")
        foreach(index RANGE ${last})
            string(JSON backend GET "${backends_json}" ${index})
            list(APPEND backends ${backend})
        endforeach()
    endif()
    mln_plugin(${id} "${backends}")
endforeach()

# The tests of the plugin API of the host. A plugin's tests in tests/host compare it with the host's built-in layer, which
# needs the headless renderer and is only run with Vulkan.
file(GLOB plugin_host_tests CONFIGURE_DEPENDS plugins/*/tests/host/*.test.cpp)
add_executable(mln-plugin-api-tests EXCLUDE_FROM_ALL
    ../test/plugin/api.test.cpp ../test/plugin/registry.test.cpp
    ../test/plugin/layout.test.cpp ../test/plugin/property_binding.test.cpp ../test/plugin/rendering.test.cpp
    $<$<BOOL:${MLN_WITH_VULKAN}>:${plugin_host_tests}>
    ../test/src/mln/test/stub_file_source.cpp
    ../vendor/googletest/googletest/src/gtest_main.cc)
target_include_directories(mln-plugin-api-tests PRIVATE ../src ../test/include ../test/src)
get_target_property(MLN_PLUGIN_TEST_LIBRARIES mbgl-core LINK_LIBRARIES)
target_link_libraries(mln-plugin-api-tests PRIVATE ${MLN_PLUGIN_LIBRARIES} mbgl-vendor-googletest ${MLN_PLUGIN_TEST_LIBRARIES})
add_dependencies(mln-plugin-tests mln-plugin-api-tests)
add_test(NAME mln-plugin-api-tests COMMAND mln-plugin-api-tests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
