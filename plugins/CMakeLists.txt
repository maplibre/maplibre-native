enable_testing()

# `mln-plugin-tests` builds the tests of every plugin, which ctest runs from the repository root.
add_custom_target(mln-plugin-tests)

get_target_property(test_libraries mbgl-core LINK_LIBRARIES)

# Every plugin in plugins/plugins, built from its directory layout:
#   shared/cpp/*.cpp, shared/include, vendor   the library mln-<id>
#   tests/*.test.cpp                           googletest tests of the library
#   tests/host/*.test.cpp                      tests with the host's renderer, which compare it with a built-in layer,
#                                              only built with Vulkan
#   render-tests                               see render_test.cpp
file(GLOB plugin_manifests CONFIGURE_DEPENDS plugins/*/plugin.json)
foreach(manifest ${plugin_manifests})
    get_filename_component(directory ${manifest} DIRECTORY)
    get_filename_component(id ${directory} NAME)
    string(REPLACE "-" "_" snake ${id})
    string(TOUPPER ${snake} upper)

    file(GLOB sources CONFIGURE_DEPENDS ${directory}/shared/cpp/*.cpp)
    add_library(mln-${id} STATIC ${sources})
    target_compile_features(mln-${id} PUBLIC cxx_std_20)
    set_target_properties(mln-${id} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_include_directories(mln-${id} PUBLIC ${directory}/shared/include PRIVATE ${directory}/vendor)
    target_link_libraries(mln-${id} PUBLIC mbgl-core)
    if(MLN_WITH_METAL)
        # A plugin may include its Metal shaders only in a Metal build.
        target_compile_definitions(mln-${id} PRIVATE MLN_${upper}_METAL=1)
    endif()

    file(GLOB tests CONFIGURE_DEPENDS ${directory}/tests/*.test.cpp)
    if(MLN_WITH_VULKAN)
        file(GLOB host_tests CONFIGURE_DEPENDS ${directory}/tests/host/*.test.cpp)
    endif()
    if(tests OR host_tests)
        add_executable(mln-${id}-tests EXCLUDE_FROM_ALL ${tests} ${host_tests}
            ../test/src/mln/test/stub_file_source.cpp ../vendor/googletest/googletest/src/gtest_main.cc)
        target_include_directories(mln-${id}-tests PRIVATE ../src ../test/include ../test/src)
        target_link_libraries(mln-${id}-tests PRIVATE mln-${id} mbgl-vendor-googletest ${test_libraries})
        add_dependencies(mln-plugin-tests mln-${id}-tests)
        add_test(NAME mln-${id}-tests COMMAND mln-${id}-tests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    endif()

    add_executable(mln-${id}-render-test EXCLUDE_FROM_ALL render_test.cpp)
    target_compile_definitions(mln-${id}-render-test PRIVATE
        MLN_PLUGIN_HEADER="${snake}.hpp" MLN_PLUGIN_REGISTER=mln_${snake}_register)
    target_link_libraries(mln-${id}-render-test PRIVATE mln-${id} mbgl-render-test)
    add_dependencies(mln-plugin-tests mln-${id}-render-test)
    add_test(NAME mln-${id}-render-test COMMAND mln-${id}-render-test --plugin-directory plugins/plugins/${id}
             WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endforeach()

# The tests of the plugin API of the host.
add_executable(mln-plugin-api-tests EXCLUDE_FROM_ALL
    ../test/plugin/api.test.cpp ../test/plugin/registry.test.cpp ../test/plugin/layout.test.cpp
    ../test/plugin/property_binding.test.cpp ../test/plugin/rendering.test.cpp
    ../test/src/mln/test/stub_file_source.cpp ../vendor/googletest/googletest/src/gtest_main.cc)
target_include_directories(mln-plugin-api-tests PRIVATE ../src ../test/include ../test/src)
target_link_libraries(mln-plugin-api-tests PRIVATE mbgl-core mbgl-vendor-googletest ${test_libraries})
add_dependencies(mln-plugin-tests mln-plugin-api-tests)
add_test(NAME mln-plugin-api-tests COMMAND mln-plugin-api-tests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
