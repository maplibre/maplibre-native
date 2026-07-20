# Set Feature State

This example demonstrates how to use feature state to create interactive maps with dynamic styling based on user interactions.

## Overview

[Feature state](https://maplibre.org/maplibre-style-spec/expressions/#feature-state) allows you to assign user-defined key-value pairs to features at runtime for styling purposes. This is useful for creating interactive maps where features change appearance based on user interactions like selection, highlighting, or other states.

Feature state is managed through the source that contains the feature: ``MLNShapeSource`` for GeoJSON data and ``MLNVectorTileSource`` for vector tiles. Features are identified by their feature identifier, so your app only needs to keep track of identifiers, not whole feature objects.

## Key Concepts

- **Feature State**: User-defined key-value pairs assigned to features at runtime
- **Feature-State Expressions**: Style expressions that access feature state values
- **Interactive Styling**: Dynamic visual changes based on user interactions

## Example Implementation

### Setting Up the Map View

<!-- include-example(FeatureStateExampleSetup) -->

```swift
import MapLibre
import SwiftUI
import UIKit

class FeatureStateExampleUIKit: UIViewController, MLNMapViewDelegate {
    private var mapView: MLNMapView!
    private var statesSource: MLNShapeSource!

    override func viewDidLoad() {
        super.viewDidLoad()

        let mapView = MLNMapView(frame: view.bounds)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.delegate = self

        // Set initial camera position to show US states
        mapView.setCenter(CLLocationCoordinate2D(latitude: 42.619626, longitude: -103.523181), zoomLevel: 3, animated: false)
        view.addSubview(mapView)

        self.mapView = mapView
    }
```

### Adding Data Source and Style Layers

<!-- include-example(FeatureStateExampleLayers) -->

```swift
func mapView(_ mapView: MLNMapView, didFinishLoading style: MLNStyle) {
        // Add US states GeoJSON source
        let statesURL = URL(string: "https://maplibre.org/maplibre-gl-js/docs/assets/us_states.geojson")!
        let statesSource = MLNShapeSource(identifier: "states", url: statesURL)
        style.addSource(statesSource)
        self.statesSource = statesSource

        // Add state fills layer with a feature-state expression for a selection effect
        let stateFillsLayer = MLNFillStyleLayer(identifier: "state-fills", source: statesSource)

        // Use a feature-state expression to change color based on selection
        let fillColorExpression = NSExpression(mglJSONObject: [
            "case",
            ["boolean", ["feature-state", "selected"], false],
            UIColor.red.withAlphaComponent(0.7), // Selected color
            UIColor.blue.withAlphaComponent(0.5), // Default color
        ])
        stateFillsLayer.fillColor = fillColorExpression

        style.addLayer(stateFillsLayer)

        // Add state borders layer with a feature-state expression for a selection effect
        let stateBordersLayer = MLNLineStyleLayer(identifier: "state-borders", source: statesSource)

        // Use a feature-state expression to change border color based on selection
        let borderColorExpression = NSExpression(mglJSONObject: [
            "case",
            ["boolean", ["feature-state", "selected"], false],
            UIColor.red, // Selected border color
            UIColor.blue, // Default border color
        ])
        stateBordersLayer.lineColor = borderColorExpression

        // Use a feature-state expression to change line width based on selection
        let borderWidthExpression = NSExpression(mglJSONObject: [
            "case",
            ["boolean", ["feature-state", "selected"], false],
            2.0,
            1.0,
        ])
        stateBordersLayer.lineWidth = borderWidthExpression

        style.addLayer(stateBordersLayer)

        // Add tap gesture recognizer to handle feature selection
        let tapGesture = UITapGestureRecognizer(target: self, action: #selector(handleMapTap(_:)))
        mapView.addGestureRecognizer(tapGesture)
    }
```

### Handling User Interactions

<!-- include-example(FeatureStateExampleInteraction) -->

```swift
@objc private func handleMapTap(_ gesture: UITapGestureRecognizer) {
        let location = gesture.location(in: mapView)
        let features = mapView.visibleFeatures(at: location, styleLayerIdentifiers: ["state-fills"])

        guard let feature = features.first, let featureID = feature.identifier.map({ "\($0)" }) else {
            return
        }

        // Toggle the selection state of the tapped feature. Selection is not
        // exclusive: any number of states can be selected at the same time.
        let isSelected = statesSource.featureState(featureID: featureID)?["selected"] as? Bool ?? false
        statesSource.setFeatureState(featureID: featureID, state: ["selected": !isSelected])

        // Show alert with feature information
        let stateName = feature.attributes["STATE_NAME"] as? String ?? "Unknown State"
        let alert = UIAlertController(
            title: "State Selected",
            message: "\(stateName) is now \(!isSelected ? "selected" : "deselected")",
            preferredStyle: .alert
        )
        alert.addAction(UIAlertAction(title: "OK", style: .default))
        present(alert, animated: true)
    }
```

### SwiftUI Integration

<!-- include-example(FeatureStateExampleSwiftUI) -->

```swift
}

struct FeatureStateExampleUIViewControllerRepresentable: UIViewControllerRepresentable {
    typealias UIViewControllerType = FeatureStateExampleUIKit

    func makeUIViewController(context _: Context) -> FeatureStateExampleUIKit {
        FeatureStateExampleUIKit()
    }

    func updateUIViewController(_: FeatureStateExampleUIKit, context _: Context) {}
}

// SwiftUI wrapper
struct FeatureStateExample: View {
    var body: some View {
        FeatureStateExampleUIViewControllerRepresentable()
            .navigationTitle("Feature State")
            .navigationBarTitleDisplayMode(.inline)
    }
}
```

## Key Features Demonstrated

### 1. Setting Feature State

```swift
// Merge state into a feature of a GeoJSON source
shapeSource.setFeatureState(featureID: "54", state: ["selected": true])

// Vector tile sources additionally take the source layer
vectorSource.setFeatureState(sourceLayerID: "states", featureID: "54", state: ["selected": true])
```

### 2. Getting Feature State

```swift
// Get the current state of a feature
let state = shapeSource.featureState(featureID: "54")
let isSelected = state?["selected"] as? Bool ?? false
```

### 3. Removing Feature State

```swift
// Remove a single state key from a feature
shapeSource.removeFeatureState(featureID: "54", stateKey: "selected")

// Remove all state from a feature
shapeSource.removeFeatureState(featureID: "54")

// Remove all feature state from the source
shapeSource.resetFeatureStates()
```

### 4. Feature-State Expressions

```swift
// Create expressions that respond to feature state
let colorExpression = NSExpression(mglJSONObject: [
    "case",
    ["boolean", ["feature-state", "selected"], false],
    UIColor.red,    // Selected color
    UIColor.blue    // Default color
])
```

## API Reference

### MLNShapeSource Feature State Methods

- ``MLNShapeSource/setFeatureStateForFeatureID:state:`` - Merge state into a feature
- ``MLNShapeSource/featureStateForFeatureID:`` - Get the current state of a feature
- ``MLNShapeSource/removeFeatureStateForFeatureID:stateKey:`` - Remove a state key from a feature
- ``MLNShapeSource/removeFeatureStateForFeatureID:`` - Remove all state from a feature
- ``MLNShapeSource/resetFeatureStates`` - Remove all feature state from the source

### MLNVectorTileSource Feature State Methods

- ``MLNVectorTileSource/setFeatureStateForSourceLayerID:featureID:state:`` - Merge state into a feature
- ``MLNVectorTileSource/featureStateForSourceLayerID:featureID:`` - Get the current state of a feature
- ``MLNVectorTileSource/removeFeatureStateForSourceLayerID:featureID:stateKey:`` - Remove a state key from a feature
- ``MLNVectorTileSource/removeFeatureStateForSourceLayerID:featureID:`` - Remove all state from a feature
- ``MLNVectorTileSource/resetFeatureStatesForSourceLayerID:`` - Remove all feature state from a source layer

## Best Practices

1. **Use meaningful state keys**: Choose descriptive names like "selected", "highlighted", "touched"
2. **Keep state values simple**: Use basic JSON types (strings, numbers, booleans)
3. **Store feature identifiers, not features**: Feature state is keyed by identifier, so keeping the identifier of e.g. the selected feature is enough
4. **Clear state when appropriate**: Remove state when features are no longer relevant
5. **Use feature-state expressions**: Create dynamic styling that responds to state changes
6. **Handle edge cases**: Always provide fallback values in expressions

## Related Examples

- [Predicates and Expressions](Predicates_and_Expressions.md) - Learn about style expressions
- [GeoJSON](GeoJSON.md) - Working with GeoJSON data sources
- [Gesture Recognizers](GestureRecognizers.md) - Handling user interactions
