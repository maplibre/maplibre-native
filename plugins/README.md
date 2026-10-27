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

`plugins/android` and `plugins/ios` hold the Android and the iOS wrapper of each plugin. They are generated: after
changing a `plugin.json` or `plugins/tools/templates`, run

```sh
node plugins/tools/generate.ts
```

`plugins/demo/android` and `plugins/demo/ios` are demo apps. They list their demos themselves and register the plugins
through their wrappers. Build the iOS app with
`bazel build //plugins/demo/ios:PluginDemo --//:renderer=metal --//:plugins=true`.
