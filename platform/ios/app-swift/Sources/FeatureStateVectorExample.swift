// #-example-code(FeatureStateVectorExampleSetup)
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

    // #-end-example-code

    // #-example-code(FeatureStateVectorExampleLayers)

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

    // #-end-example-code

    // #-example-code(FeatureStateVectorExampleInteraction)

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
    // #-end-example-code

    // #-example-code(FeatureStateVectorExampleSwiftUI)
}

struct FeatureStateVectorExampleUIViewControllerRepresentable: UIViewControllerRepresentable {
    typealias UIViewControllerType = FeatureStateVectorExampleUIKit

    func makeUIViewController(context _: Context) -> FeatureStateVectorExampleUIKit {
        FeatureStateVectorExampleUIKit()
    }

    func updateUIViewController(_: FeatureStateVectorExampleUIKit, context _: Context) {}
}

// SwiftUI wrapper
struct FeatureStateVectorExample: View {
    var body: some View {
        FeatureStateVectorExampleUIViewControllerRepresentable()
            .navigationTitle("Feature State (Vector)")
            .navigationBarTitleDisplayMode(.inline)
    }
}

// #-end-example-code
