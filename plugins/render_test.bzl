"""The render test of a plugin: plugins/render_test.cpp built for the plugin."""

load("@rules_cc//cc:defs.bzl", "cc_test")
load("//bazel:flags.bzl", "CPP_FLAGS", "MAPLIBRE_FLAGS")

def plugin_render_test(name, plugin, header, register, manifest, layer_type = None, ignore = None, data = []):
    """Runs the render tests of a manifest with the plugin registered.

    Args:
      name: The name of the test.
      plugin: The cc_library of the plugin.
      header: The header of the plugin, as included.
      register: The register function of the plugin.
      manifest: The manifest, from the repository root. A dict from a renderer config setting to a manifest limits the
        test to those renderers.
      layer_type: Only run the tests whose style has a layer of this type, among the tests of the manifest.
      ignore: A JSON file in the package of the tests to leave out, with the reason: {"render-tests/<test>": "reason"}.
      data: The files the tests read.
    """
    args = ["--manifestPath", manifest]
    compatible = []
    if type(manifest) == "dict":
        args = select({renderer: ["--manifestPath", path] for renderer, path in manifest.items()})
        compatible = select(
            {renderer: [] for renderer in manifest} | {"//conditions:default": ["@platforms//:incompatible"]},
        )
    if layer_type:
        args = args + ["--layer-type", layer_type]
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
        local_defines = ["MLN_PLUGIN_HEADER=\\\"%s\\\"" % header, "MLN_PLUGIN_REGISTER=" + register],
        tags = ["no-sandbox"],
        target_compatible_with = compatible,
        deps = [plugin, "//render-test:render-test-srcs"],
    )
