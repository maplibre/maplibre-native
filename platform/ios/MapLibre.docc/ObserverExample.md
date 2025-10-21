# Observe Low-Level Events

Learn about the ``MLNMapViewDelegate`` methods for observing map events.

> Warning: These methods are not thread-safe.

You can observe certain low-level events as they happen. Use these methods to collect metrics or investigate issues during map rendering. This feature is intended primarily for power users. We are always interested in improving observability, so if you have a special use case, feel free to [open an issue or pull request](https://github.com/maplibre/maplibre-native) to extend the types of observability methods.

## Frame Events

Observe frame rendering statistics with ``MLNMapViewDelegate/mapViewDidFinishRenderingFrame:fullyRendered:renderingStats:``.

<!-- include-example(ObserverExampleRenderingStats) -->

```swift
func mapViewDidFinishRenderingFrame(_: MLNMapView, fullyRendered _: Bool, renderingStats _: MLNRenderingStats) {}
```

See also: ``MLNMapViewDelegate/mapViewDidFinishRenderingFrame:fullyRendered:`` and ``MLNMapViewDelegate/mapViewDidFinishRenderingFrame:fullyRendered:frameEncodingTime:frameRenderingTime:``

## Shader Events

Observe shader compilation with ``MLNMapViewDelegate/mapView:shaderWillCompile:backend:defines:`` and ``MLNMapViewDelegate/mapView:shaderDidCompile:backend:defines:``.

<!-- include-example(ObserverExampleShaders) -->

```swift
func mapView(_: MLNMapView, shaderWillCompile id: Int, backend: Int, defines: String) {
        print("A new shader is being compiled - shaderID:\(id), backend type:\(backend), program configuration:\(defines)")
    }

    func mapView(_: MLNMapView, shaderDidCompile id: Int, backend: Int, defines: String) {
        print("A shader has been compiled - shaderID:\(id), backend type:\(backend), program configuration:\(defines)")
    }
```

See also: ``MLNMapViewDelegate/mapView:shaderDidFailCompile:backend:defines:``.

## Glyph Loading

Observe glyph loading events with ``MLNMapViewDelegate/mapView:glyphsWillLoad:range:`` and ``MLNMapViewDelegate/mapView:glyphsDidLoad:range:``.

<!-- include-example(ObserverExampleGlyphs) -->

```swift
func mapView(_: MLNMapView, glyphsWillLoad fontStack: [String], range: NSRange) {
        print("Glyphs are being requested for the font stack \(fontStack), ranging from \(range.location) to \(range.location + range.length)")
    }

    func mapView(_: MLNMapView, glyphsDidLoad fontStack: [String], range: NSRange) {
        print("Glyphs have been loaded for the font stack \(fontStack), ranging from \(range.location) to \(range.location + range.length)")
    }
```

See also: ``MLNMapViewDelegate/mapView:glyphsDidError:range:``.

## Tile Events

Monitor tile-related actions using the delegate method ``MLNMapViewDelegate/mapView:tileDidTriggerAction:x:y:z:wrap:overscaledZ:sourceID:`` with the ``MLNTileOperation`` type.

<!-- include-example(ObserverExampleTiles) -->

```swift
func mapView(_: MLNMapView, tileDidTriggerAction operation: MLNTileOperation,
                 x: Int,
                 y: Int,
                 z: Int,
                 wrap: Int,
                 overscaledZ: Int,
                 sourceID: String)
    {
        let tileStr = String(format: "(x: %ld, y: %ld, z: %ld, wrap: %ld, overscaledZ: %ld, sourceID: %@)",
                             x, y, z, wrap, overscaledZ, sourceID)

        switch operation {
        case MLNTileOperation.requestedFromCache:
            print("Requesting tile \(tileStr) from cache")

        case MLNTileOperation.requestedFromNetwork:
            print("Requesting tile \(tileStr) from network")

        case MLNTileOperation.loadFromCache:
            print("Loading tile \(tileStr), requested from the cache")

        case MLNTileOperation.loadFromNetwork:
            print("Loading tile \(tileStr), requested from the network")

        case MLNTileOperation.startParse:
            print("Parsing tile \(tileStr)")

        case MLNTileOperation.endParse:
            print("Completed parsing tile \(tileStr)")

        case MLNTileOperation.error:
            print("An error occured during proccessing for tile \(tileStr)")

        case MLNTileOperation.cancelled:
            print("Pending work on tile \(tileStr)")

        case MLNTileOperation.nullOp:
            print("An unknown tile operation was emitted for tile \(tileStr)")

        @unknown default:
            assertionFailure()
        }
    }
```

## Sprite Loading

Observe sprite loading events with ``MLNMapViewDelegate/mapView:spriteWillLoad:url:`` and ``MLNMapViewDelegate/mapView:spriteDidLoad:url:``.

<!-- include-example(ObserverExampleSprites) -->

```swift
func mapView(_: MLNMapView, spriteWillLoad id: String, url: String) {
        print("The sprite \(id) has been requested from \(url)")
    }

    func mapView(_: MLNMapView, spriteDidLoad id: String, url: String) {
        print("The sprite \(id) has been loaded from \(url)")
    }
```

See also: ``MLNMapViewDelegate/mapView:spriteDidError:url:``.
