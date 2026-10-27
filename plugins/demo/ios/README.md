# Plugin demo for iOS

A small SwiftUI app that shows the in-tree plugin layers on a map. It is separate from the SDK's test apps in
`platform/ios`.

| Demo | Layer | What it shows |
|---|---|---|
| N-gons | [`ngon`](../../plugins/ngon-layer) | One n-gon with controls for its corners, color, radius, rotation, opacity, blur and outline. The paint properties read the feature's properties, so an edit changes only the feature's data. The Randomize button picks new ones, and the JSON tab shows the layer's style and the feature's properties. |
| 3D models | [`gltf`](../../plugins/gltf-layer) | Cars and trees drawn from `.glb` files. A feature picks its model with `model-id`; heading and size are paint properties; buildings hide the cars through `model-occlusion`. |
| Any other plugin | | A plugin whose `plugin.json` has a `demo` is listed with its style at its camera, like in the Android demo. At the moment that is [`fill-extrusion`](../../plugins/fill-extrusion), on the OpenFreeMap Liberty style. |

The layer types only exist in a build with plugin support, and the `gltf` and `fill-extrusion` layers only have a Metal
shader, so the app only builds with `--//:renderer=metal --//:plugins=true`.

## Run it in the simulator

```sh
bazel build //plugins/demo/ios:PluginDemo --//:renderer=metal --//:plugins=true --ios_multi_cpus=sim_arm64

# Boot a simulator, install the app and launch it.
xcrun simctl boot "iPhone 17 Pro" 2>/dev/null; open -a Simulator
rm -rf /tmp/PluginDemo && unzip -oq bazel-bin/plugins/demo/ios/PluginDemo.ipa -d /tmp/PluginDemo
BUNDLE_ID=$(defaults read /tmp/PluginDemo/Payload/PluginDemo.app/Info CFBundleIdentifier)
xcrun simctl uninstall booted "$BUNDLE_ID" 2>/dev/null  # an install over the old build can keep the old binary
xcrun simctl install booted /tmp/PluginDemo/Payload/PluginDemo.app
xcrun simctl launch booted "$BUNDLE_ID"
```

The app opens on a list of demos. Launch arguments, handy for screenshots:

| Argument | Effect |
|---|---|
| `-demo models` / `-demo ngons` / `-demo fill-extrusion` | Open that demo straight away: the two demos with a view of their own by name, the others by plugin ID. |

A plugin with a `demo` in its `plugin.json` needs no code here. To show a plugin with a view of its own, write the view
and add an entry to `Demo.custom` in `Sources/DemoList.swift`.

## How the plugins get in

- `//plugins/ios:plugins` links the generated wrapper of every plugin in `plugins/plugins` (see
  [plugins/ios](../../ios)), and `PluginRegistry` registers one of them by its ID.
- A demo registers its plugin when it opens, because a style naming an unregistered layer type drops the layer. A
  plugin stays registered for the rest of the process, so `fill-extrusion` replaces the built-in layer in the demos
  opened after it until the app restarts.
- The SDK also links the `gltf` layer, for `MLNPuckPlugin`. The wrapper depends on the same library, so the plugin's
  code is in the app once.
- The list of the other demos comes from the `<id>.demo.json` files the wrappers carry, bundled through
  `//plugins/ios:demos`.
- Plugin layers have no Objective-C class, so the app adds them from JSON with
  `MLNStyle.addLayer(withJSON:belowLayerWithIdentifier:)`.

## Assets

The styles and models of the two demos with a view of their own are in [`../assets`](../assets), shared with the other
demo apps; its README describes how to regenerate the models. `ios_application` bundles them at the root of the app, so
`asset://car-red.glb` in a style finds the model.
