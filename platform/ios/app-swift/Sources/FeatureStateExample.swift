// #-example-code(FeatureStateExampleSetup)
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

    // #-end-example-code

    // #-example-code(FeatureStateExampleLayers)

    func mapView(_ mapView: MLNMapView, didFinishLoading style: MLNStyle) {
        // Add US states GeoJSON source
        let statesURL = URL(string: "https://maplibre.org/maplibre-gl-js/docs/assets/us_states.geojson")!
        let statesSource = MLNShapeSource(identifier: "states", url: statesURL)
        style.addSource(statesSource)

        // Add state fills layer with feature-state expressions for highlighting and selection effects
        let stateFillsLayer = MLNFillStyleLayer(identifier: "state-fills", source: statesSource)

        // Use feature-state expression to change color based on selection
        let fillColorExpression = NSExpression(mglJSONObject: [
            "case",
            ["boolean", ["feature-state", "selected"], false],
            UIColor.red.withAlphaComponent(0.7), // Selected color
            UIColor.blue.withAlphaComponent(0.5), // Default color
        ])
        stateFillsLayer.fillColor = fillColorExpression

        // Use feature-state expression to change opacity when highlighted
        // This expression checks if the feature has a "highlighted" state set to true
        let highlightedExpression = NSExpression(mglJSONObject: [
            "case",
            ["boolean", ["feature-state", "highlighted"], false],
            1.0,
            0.5,
        ])
        stateFillsLayer.fillOpacity = highlightedExpression

        style.addLayer(stateFillsLayer)

        // Add state borders layer with feature-state expressions for highlighting and selection effects
        let stateBordersLayer = MLNLineStyleLayer(identifier: "state-borders", source: statesSource)

        // Use feature-state expression to change border color based on selection
        let borderColorExpression = NSExpression(mglJSONObject: [
            "case",
            ["boolean", ["feature-state", "selected"], false],
            UIColor.red, // Selected border color
            UIColor.blue, // Default border color
        ])
        stateBordersLayer.lineColor = borderColorExpression

        // Use feature-state expression to change line width when highlighted
        let borderWidthExpression = NSExpression(mglJSONObject: [
            "case",
            ["boolean", ["feature-state", "highlighted"], false],
            2.0,
            1.0,
        ])
        stateBordersLayer.lineWidth = borderWidthExpression

        style.addLayer(stateBordersLayer)

        // Add tap gesture recognizer to handle feature selection
        let tapGesture = UITapGestureRecognizer(target: self, action: #selector(handleMapTap(_:)))
        mapView.addGestureRecognizer(tapGesture)
    }

    // #-end-example-code

    // #-example-code(FeatureStateExampleInteraction)

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
    // #-end-example-code

    // #-example-code(FeatureStateExampleSwiftUI)
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

// #-end-example-code
