# MapLibre Native OpenHarmony Sample

Build and run the sample with the public OpenHarmony SDK and
[Oniro emulator](https://docs.oniroproject.org/device-development/developer-boards/emulator/).

## Setup

On Linux, install Node.js 20+, JDK 21, Ninja, and QEMU, then install
[Oniro App Builder](https://github.com/eclipse-oniro4openharmony/oniro-app-builder)
and the OpenHarmony 6.1 tools:

```sh
npm install -g @oniroproject/oniro-app@0.11.0
oniro-app sdk install 6.1
oniro-app cmdtools install
oniro-app emulator install
```

## Build and run

From the repository root:

```sh
git submodule update --init --recursive
cd platform/ohos/sample
oniro-app sign
oniro-app build --product opengl --mode release
oniro-app emulator start --wait-for-hdc 300
```

Unlock the emulator screen, then install and launch the sample:

```sh
oniro-app app install --hap entry/build/opengl/outputs/opengl/entry-opengl-signed.hap
oniro-app app launch
```

Use `opengl` for the emulator; the `default` product builds Vulkan.
Keep generated signing changes to `build-profile.json5` uncommitted.
