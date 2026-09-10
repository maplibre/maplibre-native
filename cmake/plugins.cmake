if(MLN_WITH_PLUGINS)
    target_compile_definitions(mbgl-core PUBLIC MLN_WITH_PLUGINS=1)
    target_sources(mbgl-core PRIVATE
        ${PROJECT_SOURCE_DIR}/include/mln/plugin/plugin_api.h
        ${PROJECT_SOURCE_DIR}/src/mln/plugin/plugin_registry.hpp
        ${PROJECT_SOURCE_DIR}/src/mln/plugin/plugin_registry.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/plugin/plugin_shader.hpp
        ${PROJECT_SOURCE_DIR}/src/mln/plugin/plugin_shader.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/plugin/plugin_style_layer_factory.hpp
        ${PROJECT_SOURCE_DIR}/src/mln/plugin/plugin_style_layer_factory.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/layout/plugin_layout.hpp
        ${PROJECT_SOURCE_DIR}/src/mln/layout/plugin_layout.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/plugin_bucket.hpp
        ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/plugin_bucket.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/style/layers/plugin_style_layer.hpp
        ${PROJECT_SOURCE_DIR}/src/mln/style/layers/plugin_style_layer.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/style/plugin_property.hpp
        ${PROJECT_SOURCE_DIR}/src/mln/style/plugin_property.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_plugin_style_layer.hpp
        ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_plugin_style_layer.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/plugin_layer_tweaker.hpp
        ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/plugin_layer_tweaker.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/plugin/plugin_drawable_data.hpp
    )
endif()
