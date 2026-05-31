# HarmonyOS

This platform support is experimental. It targets the HarmonyOS native SDK, not
yet OpenHarmony.

Public OpenHarmony platform documentation is available in the
[OpenHarmony docs repository](https://github.com/openharmony/docs).

See [the sample readme](./sample/README.md) to build and run the NAPI/XComponent
integration example in DevEco Studio.

## CMake Build

When invoking CMake directly from this repository root, set the native SDK paths
before using the HarmonyOS presets:

```sh
export HMOS_SDK_NATIVE=/path/to/hms/native
export OHOS_SDK_NATIVE=/path/to/openharmony/native

cmake --preset harmonyos-opengl
cmake --build --preset harmonyos-opengl

cmake --preset harmonyos-vulkan
cmake --build --preset harmonyos-vulkan
```

The HarmonyOS toolchain wraps the OHOS base toolchain, so both
`HMOS_SDK_NATIVE` and `OHOS_SDK_NATIVE` are required. The presets are disabled
until both variables are set.

DevEco Studio and Hvigor provide these paths from the configured HarmonyOS SDK
when building `platform/ohos/sample`.
