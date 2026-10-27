"""The render test of a plugin: plugins/render_test.cpp built for the plugin."""

load("@rules_cc//cc:defs.bzl", "cc_test")
load("//bazel:flags.bzl", "CPP_FLAGS", "MAPLIBRE_FLAGS")

def plugin_render_test(name, data):
    """Runs the render tests of the plugin of this package, see render_test.cpp.

    The plugin is the cc_library, header and register function that its directory <id> names, see plugins/README.md.

    Args:
      name: The name of the test.
      data: The files the tests read.
    """
    id = native.package_name().split("/")[-1]
    snake = id.replace("-", "_")
    cc_test(
        name = name,
        timeout = "long",
        srcs = ["//plugins:render_test.cpp"],
        args = ["--plugin-directory", native.package_name()],
        copts = CPP_FLAGS + MAPLIBRE_FLAGS,
        data = data + [":plugin.json"],
        local_defines = ["MLN_PLUGIN_HEADER=\\\"%s.hpp\\\"" % snake, "MLN_PLUGIN_REGISTER=mln_%s_register" % snake],
        tags = ["no-sandbox"],
        # Bazel builds the OpenGL (drawable) and Metal renderers.
        target_compatible_with = select({
            "//:drawable_renderer": [],
            "//:metal_renderer": [],
            "//conditions:default": ["@platforms//:incompatible"],
        }),
        deps = [":" + id, "//render-test:render-test-srcs"],
    )
