# HarmonyOS

This platform support is experimental. It targets the HarmonyOS native SDK, not
yet OpenHarmony.

The `platform/ohos/sample` app is the easiest way to build and run the current
NAPI/XComponent integration in DevEco Studio.

## Sample App

Open `platform/ohos/sample` in DevEco Studio. See
`platform/ohos/sample/README.md` for local signing and renderer selection.

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
