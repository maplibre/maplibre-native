# Set Feature State

This example demonstrates how to use feature state to create interactive maps with dynamic styling based on user interactions.

## Overview

[Feature state](https://maplibre.org/maplibre-style-spec/expressions/#feature-state) allows you to assign user-defined key-value pairs to features at runtime for styling purposes. This is useful for creating interactive maps where features change appearance based on user interactions like selection, hover, or other states.

@Video(
   source: "FeatureState.mp4",
   poster: "FeatureState.png",
   alt: "A video showing how tapping on US states toggles their feature-state values."
)

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

        // Add state fills layer with feature-state expressions for hover and selection effects
        let stateFillsLayer = MLNFillStyleLayer(identifier: "state-fills", source: statesSource)

        // Use feature-state expression to change color based on selection
        let fillColorExpression = NSExpression(mglJSONObject: [
            "case",
            ["boolean", ["feature-state", "selected"], false],
            UIColor.red.withAlphaComponent(0.7), // Selected color
            UIColor.blue.withAlphaComponent(0.5)  // Default color
        ])
        stateFillsLayer.fillColor = fillColorExpression

        // Use feature-state expression to change opacity on hover
        // This expression checks if the feature has a "hover" state set to true
        let hoverExpression = NSExpression(mglJSONObject: [
            "case",
            ["boolean", ["feature-state", "hover"], false],
            1.0,
            0.5
        ])
        stateFillsLayer.fillOpacity = hoverExpression

        style.addLayer(stateFillsLayer)

        // Add state borders layer with feature-state expressions for hover and selection effects
        let stateBordersLayer = MLNLineStyleLayer(identifier: "state-borders", source: statesSource)

        // Use feature-state expression to change border color based on selection
        let borderColorExpression = NSExpression(mglJSONObject: [
            "case",
            ["boolean", ["feature-state", "selected"], false],
            UIColor.red, // Selected border color
            UIColor.blue // Default border color
        ])
        stateBordersLayer.lineColor = borderColorExpression

        // Use feature-state expression to change line width on hover
        let borderWidthExpression = NSExpression(mglJSONObject: [
            "case",
            ["boolean", ["feature-state", "hover"], false],
            2.0,
            1.0
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

        if let feature = features.first {
            // Toggle selection state
            let currentState = mapView.getFeatureState(feature, sourceID: "states")
            let isSelected = currentState?["selected"] as? Bool ?? false

            mapView.setFeatureState(feature, sourceID: "states", state: ["selected": !isSelected])

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
    }
```

### SwiftUI Integration

<!-- include-example(FeatureStateExampleSwiftUI) -->

```swift
}

struct FeatureStateExampleUIViewControllerRepresentable: UIViewControllerRepresentable {
    typealias UIViewControllerType = FeatureStateExampleUIKit

    func makeUIViewController(context: Context) -> FeatureStateExampleUIKit {
        FeatureStateExampleUIKit()
    }

    func updateUIViewController(_ uiViewController: FeatureStateExampleUIKit, context: Context) {}
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
// Set feature state for a specific feature
mapView.setFeatureState(feature, sourceID: "states", state: ["selected": true])

// Set feature state using explicit identifiers
mapView.setFeatureStateForSource("states", sourceLayer: nil, featureID: "54", state: ["selected": true])
```

### 2. Getting Feature State
```swift
// Get current state of a feature
let currentState = mapView.getFeatureState(feature, sourceID: "states")
let isSelected = currentState?["selected"] as? Bool ?? false

// Get state using explicit identifiers
let state = mapView.getFeatureStateForSource("states", sourceLayer: nil, featureID: "54")
```

### 3. Removing Feature State
```swift
// Remove specific state key
mapView.removeFeatureState(feature, sourceID: "states", stateKey: "selected")

// Remove all state for a feature
mapView.removeFeatureState(feature, sourceID: "states", stateKey: nil)
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

### MLNMapView Feature State Methods

- ``MLNMapView/setFeatureState:sourceID:state:`` - Set state for a feature
- ``MLNMapView/setFeatureStateForSource:sourceLayer:featureID:state:`` - Set state with explicit identifiers
- ``MLNMapView/getFeatureState:sourceID:`` - Get current state of a feature
- ``MLNMapView/getFeatureStateForSource:sourceLayer:featureID:`` - Get state with explicit identifiers
- ``MLNMapView/removeFeatureState:sourceID:stateKey:`` - Remove state from a feature
- ``MLNMapView/removeFeatureStateForSource:sourceLayer:featureID:stateKey:`` - Remove state with explicit identifiers

## Best Practices

1. **Use meaningful state keys**: Choose descriptive names like "selected", "hover", "highlighted"
2. **Keep state values simple**: Use basic JSON types (strings, numbers, booleans)
3. **Clear state when appropriate**: Remove state when features are no longer relevant
4. **Use feature-state expressions**: Create dynamic styling that responds to state changes
5. **Handle edge cases**: Always provide fallback values in expressions

## Related Examples

- [Predicates and Expressions](Predicates_and_Expressions.md) - Learn about style expressions
- [GeoJSON](GeoJSON.md) - Working with GeoJSON data sources
- [Gesture Recognizers](GestureRecognizers.md) - Handling user interactions
