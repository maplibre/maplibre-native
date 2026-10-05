// #-example-code(FeatureStateExampleSetup)
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

    // #-end-example-code

    // #-example-code(FeatureStateExampleLayers)

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

    // #-end-example-code

    // #-example-code(FeatureStateExampleInteraction)

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
