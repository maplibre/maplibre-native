"""The render test of a plugin: plugins/render_test.cpp built for the plugin."""

load("@rules_cc//cc:defs.bzl", "cc_test")
load("//bazel:flags.bzl", "CPP_FLAGS", "MAPLIBRE_FLAGS")

def plugin_render_test(name, manifest, plugin_layers = False, ignore = None, data = []):
    """Runs the render tests of a manifest with the plugin of this package registered.

    The plugin is the cc_library, header and register function that its directory <id> names, see plugins/README.md.

    Args:
      name: The name of the test.
      manifest: The manifest, from the repository root. A dict from a renderer config setting to a manifest limits the
        test to those renderers.
      plugin_layers: Only run the tests whose style has a layer of a type that the plugin registers, among the tests of
        the manifest.
      ignore: A JSON file in the package of the tests to leave out, with the reason: {"render-tests/<test>": "reason"}.
      data: The files the tests read.
    """
    id = native.package_name().split("/")[-1]
    snake = id.replace("-", "_")
    if type(manifest) == "dict":
        args = select({renderer: ["--manifestPath", path] for renderer, path in manifest.items()})
        compatible = select(
            {renderer: [] for renderer in manifest} | {"//conditions:default": ["@platforms//:incompatible"]},
        )
    else:
        args = ["--manifestPath", manifest]
        compatible = []
    if plugin_layers:
        args = args + ["--plugin-layers"]
    if ignore:
        args = args + ["--ignore", native.package_name() + "/" + ignore]
        data = data + [ignore]
    cc_test(
        name = name,
        timeout = "long",
        srcs = ["//plugins:render_test.cpp"],
        args = args,
        copts = CPP_FLAGS + MAPLIBRE_FLAGS,
        data = data,
        local_defines = ["MLN_PLUGIN_HEADER=\\\"%s.hpp\\\"" % snake, "MLN_PLUGIN_REGISTER=mln_%s_register" % snake],
        tags = ["no-sandbox"],
        target_compatible_with = compatible,
        deps = [":" + id, "//render-test:render-test-srcs"],
    )
