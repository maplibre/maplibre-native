# OpenHarmony

This platform support is experimental. It targets the
[OpenHarmony](https://en.wikipedia.org/wiki/OpenHarmony) family of operating
systems, including [HarmonyOS](https://en.wikipedia.org/wiki/HarmonyOS).

Public OpenHarmony platform documentation is available in the
[OpenHarmony docs repository](https://github.com/openharmony/docs).

See [the sample readme](./sample/README.md) to build and run the NAPI/XComponent
integration example in DevEco Studio.

## CMake Build

When invoking CMake directly from this repository root, set the native SDK paths
before using the HarmonyOS presets:

```sh
export OHOS_SDK_NATIVE=/path/to/openharmony/native

cmake --preset harmonyos-opengl
cmake --build --preset harmonyos-opengl

cmake --preset harmonyos-vulkan
cmake --build --preset harmonyos-vulkan
```

The presets use the OpenHarmony SDK from `OHOS_SDK_NATIVE`. They are disabled
until that variable is set.

DevEco Studio and Hvigor provide the native SDK path from the configured SDK
when building `platform/ohos/sample`.
