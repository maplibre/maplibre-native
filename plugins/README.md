# Plugins

A plugin is a directory in `plugins/plugins` with its platform-independent code, tests, render tests and a
`plugin.json`:

```json
{
  "description": "What the plugin does",
  "backends": ["vulkan", "metal"]
}
```

`backends` (default: all) is optional. Everything else follows from the directory name `<id>`: the code is
`shared/cpp/*.cpp` with the header `shared/include/<id with underscores>.hpp` and the function
`mln_<id with underscores>_register`.

The builds follow the directory too. CMake builds every plugin (`mln_plugin` in `plugins/CMakeLists.txt`) and the
`BUILD.bazel` of a plugin only calls `mln_plugin()` (`plugins/plugin.bzl`). `tests/*.test.cpp` are googletest tests, and
a test in `tests/host` that needs the host runs with the plugin API tests on Vulkan. The render tests are the plugin's
own with `render-tests/manifest.json`, or else the repository's tests of the built-in layer types that the plugin
implements, without the tests in `render-tests/ignores.json`.

A plugin cannot register a layer type with the name of a built-in one, so the layer types of the plugins here are named
`org.maplibre.<name>`. A layer type `org.maplibre.<type>` implements the built-in layer type `<type>`: its render tests
are the repository's tests of `<type>`, run with the plugin's layer type in its place, which checks that the plugin
renders like the built-in layer.

`plugins/android` and `plugins/ios` hold the Android and the iOS wrapper of each plugin. They are generated: after
changing a `plugin.json` or `plugins/tools/templates`, run

```sh
node plugins/tools/generate.ts
```

`plugins/demo/android` and `plugins/demo/ios` are demo apps. They list their demos themselves and register the plugins
through their wrappers. Build the iOS app with
`bazel build //plugins/demo/ios:PluginDemo --//:renderer=metal --//:plugins=true`.
