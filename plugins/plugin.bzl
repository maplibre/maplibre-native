"""A plugin in plugins/plugins/<id>, built from its directory layout."""

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load("//bazel:flags.bzl", "CPP_FLAGS")
load("//plugins:render_test.bzl", "plugin_render_test")

def mln_plugin():
    """Builds the plugin of this package, which is the directory plugins/plugins/<id>.

    shared/cpp/*.cpp, shared/include and vendor make the cc_library <id>. tests/*.test.cpp are googletest tests of
    the library. render-tests are the render tests of the plugin, see render_test.cpp.
    """
    id = native.package_name().split("/")[-1]
    only_with_plugins = select({
        "//:plugins_enabled": [],
        "//conditions:default": ["@platforms//:incompatible"],
    })

    cc_library(
        name = id,
        srcs = native.glob(["shared/cpp/*.cpp", "shared/cpp/*.hpp", "vendor/**/*.hpp"], allow_empty = True),
        hdrs = native.glob(["shared/include/*.hpp"]),
        copts = CPP_FLAGS,
        # A plugin may include its Metal shaders only in a Metal build: MLN_<ID>_METAL.
        defines = select({
            "//:metal_renderer": ["MLN_%s_METAL=1" % id.replace("-", "_").upper()],
            "//conditions:default": [],
        }),
        includes = ["shared/include"] + (["vendor"] if native.glob(["vendor/**"], allow_empty = True) else []),
        target_compatible_with = only_with_plugins,
        # The plugin itself has no renderer dependency: all interaction is C ABI.
        visibility = ["//visibility:public"],
        deps = ["//:mln-plugin-api"],
    )

    tests = native.glob(["tests/*.test.cpp"], allow_empty = True)
    if tests:
        cc_test(
            name = "unit-tests",
            srcs = tests,
            copts = CPP_FLAGS,
            deps = [":" + id, "@googletest//:gtest_main"],
        )

    # The plugin's own render tests, or else the repository's.
    data = native.glob(["render-tests/**"], exclude = ["render-tests/results/**"], allow_empty = True)
    if native.glob(["render-tests/manifest.json"], allow_empty = True):
        data = data + ["render-tests/results/.gitkeep"]
    else:
        data = data + ["//metrics:render-test-files"]
    plugin_render_test(name = "render-test", data = data)
