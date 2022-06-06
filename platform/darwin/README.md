# Darwin

The code in the Darwin platform targets Apple platforms but is not specific
to iOS or macOS. This code is not distributed as an SDK in itself, but is required
by iOS and macOS builds of MapLibre GL Native and ultimately by the
[MapLibre Maps SDK for iOS](https://github.com/maplibre/maplibre-gl-native/tree/main/platform/ios)
and [MapLibre Maps SDK for macOS](https://github.com/maplibre/maplibre-gl-native/tree/main/platform/macos).

These files depend on the Foundation and Core Foundation frameworks but do not
depend on iOS- or macOS–specific frameworks, such as UIKit or AppKit. Any
non-cross-platform code is guarded by TargetConditionals.h macros.
