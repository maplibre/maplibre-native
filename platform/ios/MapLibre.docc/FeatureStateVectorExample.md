# Set Feature State (Vector Source)

This example demonstrates feature state on a ``MLNVectorTileSource``, toggling the appearance of countries in a vector tileset as they are tapped.

## Overview

[Feature state](https://maplibre.org/maplibre-style-spec/expressions/#feature-state) allows you to assign user-defined key-value pairs to features at runtime for styling purposes. It works the same way for GeoJSON and vector tile sources — see <doc:FeatureStateExample> for the GeoJSON version — but a vector source differs in two important ways:

1. **A source layer is required.** Vector tiles are organized into named source layers, so every feature-state call takes a `sourceLayerID`. A ``MLNShapeSource`` has a single implicit layer and therefore omits it.
2. **Features must have an `id`.** Feature state is keyed by feature identifier, so the target features must carry an `id` in the underlying vector tile data. The [MapLibre demo tiles](https://demotiles.maplibre.org) used here assign one per country, so selection binds correctly. Note that MapLibre Native does not support promoting a feature property to an `id` at runtime, so the tileset itself must provide identifiers.

## Example Implementation

### Setting Up the Map View

<!-- include-example(FeatureStateVectorExampleSetup) -->

```swift
import MapLibre
import SwiftUI
import UIKit

class FeatureStateVectorExampleUIKit: UIViewController, MLNMapViewDelegate {
    private var mapView: MLNMapView!
    private var countriesSource: MLNVectorTileSource!

    // The source layer that holds the country polygons in the demo tiles.
    private let countriesSourceLayer = "countries"

    override func viewDidLoad() {
        super.viewDidLoad()

        // The MapLibre demo tiles are a world map of country polygons.
        let mapView = MLNMapView(frame: view.bounds, styleURL: DEMOTILES_STYLE)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.delegate = self

        mapView.setCenter(CLLocationCoordinate2D(latitude: 20, longitude: 0), zoomLevel: 1, animated: false)
        view.addSubview(mapView)

        self.mapView = mapView
    }
```

### Adding the Vector Source and Style Layers

<!-- include-example(FeatureStateVectorExampleLayers) -->

```swift
func mapView(_ mapView: MLNMapView, didFinishLoading style: MLNStyle) {
        // Add the demo tiles as a vector source, described by its TileJSON.
        let countriesSource = MLNVectorTileSource(
            identifier: "countries-source",
            configurationURL: URL(string: "https://demotiles.maplibre.org/tiles/tiles.json")!
        )
        style.addSource(countriesSource)
        self.countriesSource = countriesSource

        // Fill layer: transparent by default so the basemap shows through, and
        // highlighted only where the "selected" feature state is set.
        let fillsLayer = MLNFillStyleLayer(identifier: "country-fills", source: countriesSource)
        // A vector style layer must be told which source layer to draw.
        fillsLayer.sourceLayerIdentifier = countriesSourceLayer
        fillsLayer.fillColor = NSExpression(forConstantValue: UIColor.red)
        fillsLayer.fillOpacity = NSExpression(mglJSONObject: [
            "case",
            ["boolean", ["feature-state", "selected"], false],
            0.5, // Selected
            0.0, // Default (transparent)
        ])
        style.addLayer(fillsLayer)

        // Border layer: a thin border everywhere, thicker and red where selected.
        let bordersLayer = MLNLineStyleLayer(identifier: "country-borders", source: countriesSource)
        bordersLayer.sourceLayerIdentifier = countriesSourceLayer
        bordersLayer.lineColor = NSExpression(mglJSONObject: [
            "case",
            ["boolean", ["feature-state", "selected"], false],
            UIColor.red,
            UIColor.gray,
        ])
        bordersLayer.lineWidth = NSExpression(mglJSONObject: [
            "case",
            ["boolean", ["feature-state", "selected"], false],
            2.0,
            0.5,
        ])
        style.addLayer(bordersLayer)

        // Add tap gesture recognizer to handle feature selection
        let tapGesture = UITapGestureRecognizer(target: self, action: #selector(handleMapTap(_:)))
        mapView.addGestureRecognizer(tapGesture)
    }
```

### Handling Selection

<!-- include-example(FeatureStateVectorExampleInteraction) -->

```swift
@objc private func handleMapTap(_ gesture: UITapGestureRecognizer) {
        let location = gesture.location(in: mapView)
        let features = mapView.visibleFeatures(at: location, styleLayerIdentifiers: ["country-fills"])

        guard let feature = features.first, let featureID = feature.identifier.map({ "\($0)" }) else {
            return
        }

        // Toggle the selection state of the tapped country. Selection is not
        // exclusive: any number of countries can be selected at the same time.
        //
        // Note the sourceLayerID argument, which is required for a vector source.
        let isSelected = countriesSource.featureState(
            sourceLayerID: countriesSourceLayer,
            featureID: featureID
        )?["selected"] as? Bool ?? false
        countriesSource.setFeatureState(
            sourceLayerID: countriesSourceLayer,
            featureID: featureID,
            state: ["selected": !isSelected]
        )

        let countryName = feature.attributes["NAME"] as? String ?? "Unknown"
        let alert = UIAlertController(
            title: "Country Selected",
            message: "\(countryName) is now \(!isSelected ? "selected" : "deselected")",
            preferredStyle: .alert
        )
        alert.addAction(UIAlertAction(title: "OK", style: .default))
        present(alert, animated: true)
    }
```

## Key Differences from a GeoJSON Source

### Setting Feature State

```swift
// GeoJSON shape source — no source layer
shapeSource.setFeatureState(featureID: "54", state: ["selected": true])

// Vector tile source — source layer required
vectorSource.setFeatureState(sourceLayerID: "countries", featureID: "54", state: ["selected": true])
```

### Reading and Removing Feature State

```swift
// Read
let state = vectorSource.featureState(sourceLayerID: "countries", featureID: "54")

// Remove a single key
vectorSource.removeFeatureState(sourceLayerID: "countries", featureID: "54", stateKey: "selected")

// Remove all state from a feature
vectorSource.removeFeatureState(sourceLayerID: "countries", featureID: "54")

// Remove all feature state from the source layer
vectorSource.resetFeatureStates(sourceLayerID: "countries")
```

## API Reference

### MLNVectorTileSource Feature State Methods

- ``MLNVectorTileSource/setFeatureStateForSourceLayerID:featureID:state:`` - Merge state into a feature
- ``MLNVectorTileSource/featureStateForSourceLayerID:featureID:`` - Get the current state of a feature
- ``MLNVectorTileSource/removeFeatureStateForSourceLayerID:featureID:stateKey:`` - Remove a state key from a feature
- ``MLNVectorTileSource/removeFeatureStateForSourceLayerID:featureID:`` - Remove all state from a feature
- ``MLNVectorTileSource/resetFeatureStatesForSourceLayerID:`` - Remove all feature state from a source layer

## Related Examples

- <doc:FeatureStateExample> - The same interaction backed by a GeoJSON source
