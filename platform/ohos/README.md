# OpenHarmony

This platform support is experimental. It targets the
[OpenHarmony](https://en.wikipedia.org/wiki/OpenHarmony) family of operating
systems, including [HarmonyOS](https://en.wikipedia.org/wiki/HarmonyOS).

Public OpenHarmony platform documentation is available in the
[OpenHarmony docs repository](https://github.com/openharmony/docs).

See [the sample readme](./sample/README.md) to build and run the NAPI/XComponent
integration example with the public SDK and Oniro emulator. No Huawei account is
required.

## CMake Build

Download the [OpenHarmony 6.1 SDK](https://github.com/openharmony-rs/ohos-sdk/releases/tag/v6.1)
for your host and extract its `native` component. With Ninja installed, run from
this repository root:

```sh
export OHOS_SDK_NATIVE=/path/to/openharmony/native
export PATH="$OHOS_SDK_NATIVE/build-tools/cmake/bin:$PATH"
git submodule update --init --recursive

cmake --preset harmonyos-opengl
cmake --build --preset harmonyos-opengl

cmake --preset harmonyos-vulkan
cmake --build --preset harmonyos-vulkan
```

The presets build the core static library.
