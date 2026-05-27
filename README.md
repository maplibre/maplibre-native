# Custom MapLibre Rendering Engine

An experimental fork of **[MapLibre Native](https://github.com/maplibre/maplibre-native)** extended for **3D map rendering on Android**: glTF/GLB models, cinematic horizon views, sky rendering, soft building edges, and navigation-puck fixes at high pitch.

> **Upstream:** [maplibre/maplibre-native](https://github.com/maplibre/maplibre-native) · **Android SDK base:** 13.0.0 · **Sync guide:** [UPSTREAM.md](./UPSTREAM.md)

[![MapLibre Native](https://img.shields.io/badge/upstream-MapLibre%20Native-blue?style=flat-square&logo=github)](https://github.com/maplibre/maplibre-native)
[![License](https://img.shields.io/badge/license-BSD--2--Clause-green?style=flat-square)](./LICENSE.md)

## Relationship to MapLibre Native

| | [Official MapLibre Native](https://github.com/maplibre/maplibre-native) | This repo |
|---|--------|-----------|
| Purpose | Production cross-platform map SDK | Experimental Android 3D rendering |
| Maintained by | MapLibre organization | Independent fork |
| Docs | [maplibre.org/maplibre-native](https://maplibre.org/maplibre-native/) | [ACCOMPLISHMENTS.md](./ACCOMPLISHMENTS.md) |

This project is **not** affiliated with the MapLibre organization. It retains the [BSD 2-Clause license](./LICENSE.md) and builds on the upstream codebase. See [FORK.md](./FORK.md) for lineage and [UPSTREAM.md](./UPSTREAM.md) for how to fetch and compare against official releases.

```bash
git remote add upstream https://github.com/maplibre/maplibre-native.git   # one-time
git fetch upstream
./scripts/upstream-status.sh                                               # compare versions & custom files
```

## Features

- **glTF/GLB 3D models** on the map via `GltfModelLayer`
- **70° max pitch** and wider **field of view** (0.95 rad) for horizon-style views
- **Sky band** above the horizon when pitched
- **Soft bevel shading** on fill-extrusion buildings
- **Horizon-aware culling** so 3D content and symbols behave correctly at steep tilt
- **Navigation puck fix** at high zoom / moderate pitch

See **[ACCOMPLISHMENTS.md](./ACCOMPLISHMENTS.md)** for the full changelog, architecture, API usage, and file reference.

## Quick Start (Android Test App)

### Prerequisites

- Android Studio (recent)
- Android NDK (installed via SDK Manager)
- JDK 17+

### Build & run

```bash
cd platform/android
./gradlew :MapLibreAndroidTestApp:assembleDebug
adb install MapLibreAndroidTestApp/build/outputs/apk/debug/MapLibreAndroidTestApp-debug.apk
```

Or open `platform/android` in Android Studio and run **MapLibreAndroidTestApp**.

The app launches into a 3D model explorer. Add your own `.glb` files under:

```
platform/android/MapLibreAndroidTestApp/src/main/assets/glbModel/
```

…and configure placements in `assets/gltf_model_placements.json` (no Kotlin changes required).

### Map style

The demo uses [OpenFreeMap Liberty](https://tiles.openfreemap.org/) — a free public vector style. No API key required.

## Using `GltfModelLayer` in your app

```java
byte[] data = getAssets().open("glbModel/model.glb").readAllBytes();
GltfModelLayer layer = new GltfModelLayer(
    "my-model",
    data,
    new LatLng(37.7749, -122.4194),
    1.0f,
    90f, 0f, 0f
);
maplibreMap.getStyle().addLayer(layer);
```

Build the SDK from this repo and depend on the local `:MapLibreAndroid` module, or publish the AAR yourself.

## License

MapLibre Native is licensed under the [BSD 2-Clause License](./LICENSE.md). This fork retains the same license. See [FORK.md](./FORK.md) for upstream attribution and [UPSTREAM.md](./UPSTREAM.md) for staying in sync with [maplibre/maplibre-native](https://github.com/maplibre/maplibre-native).
