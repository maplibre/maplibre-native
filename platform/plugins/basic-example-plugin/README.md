# Basic Plugin Example

A minimal cross-platform plugin demonstrating the MapLibre plugin architecture.

## Overview

This example shows how to build a basic cross platform plugin that interacts with maplibre's c++ core API. Bindings from native to ios and android are created using objc++ and maplibre's own JNI system. With this, you can build out your implementation directly in native and only need to then expose a (hopefully) thin set of bindings to have ios and android clients call into.

## Usage

### iOS (Swift/ObjC)

```swift
import MLNBasicPluginExample

let plugin = MLNBasicPluginExample()
let mapView = MLNMapView(frame: frame, styleURL: nil, plugins: [plugin])

// Call custom method
plugin.showSanFrancisco()
```

### Android (Kotlin)

```kotlin
val plugin = BasicPluginExample()
mapView.addPluginByPtr(plugin.ptr)

// Call custom method
plugin.showSanFrancisco()
```

## Structure

```
basic-example-plugin/
├── BasicPluginExample.hpp    # C++ plugin interface
├── BasicPluginExample.cpp    # C++ implementation
├── darwin/
│   ├── MLNBasicPluginExample.h   # ObjC public header
│   └── MLNBasicPluginExample.mm  # ObjC bridge
└── android/
    └── basic_plugin_example_jni.cpp  # JNI bindings
```

## Plugin Lifecycle

1. Plugin created by platform code
2. `onLoad()` called when map is ready
3. Map observer callbacks fired during map lifecycle
4. `onUnload()` called before map destruction
