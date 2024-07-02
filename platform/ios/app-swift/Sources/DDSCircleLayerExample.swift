
import MapLibre
import SwiftUI
import UIKit

// #-example-code(DDSCircleLayerExample)
class DDSCircleLayerExample: UIViewController, MLNMapViewDelegate {
    var mapView: MLNMapView!

    override func viewDidLoad() {
        super.viewDidLoad()

        mapView = MLNMapView(frame: view.bounds, styleURL: AMERICANA_STYLE)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.tintColor = .darkGray

        // Set the map’s center coordinate and zoom level.
        let innsbruck = CLLocationCoordinate2D(latitude: 47.26497, longitude: 11.4088)
        mapView.setCenter(innsbruck, animated: false)
        mapView.zoomLevel = 14

        mapView.delegate = self
        view.addSubview(mapView)
    }

    // Wait until the style is loaded before modifying the map style.
    func mapView(_: MLNMapView, didFinishLoading style: MLNStyle) {
        let source = MLNVectorTileSource(identifier: "demotiles", configurationURL: URL(string: "https://demotiles.maplibre.org/tiles-omt/tiles.json")!)

        style.addSource(source)

        let layer = MLNCircleStyleLayer(identifier: "poi-shop-style", source: source)

        layer.sourceLayerIdentifier = "poi"
        layer.predicate = NSPredicate(format: "class == %@", "shop")

        // Style the circle layer color based on the rank
        layer.circleColor = NSExpression(mglJSONObject: ["step", ["get", "rank"], 0, "red", 10, "green", 20, "blue", 30, "purple", 40, "yellow"] as [Any])
        layer.circleRadius = NSExpression(forConstantValue: 3)

        style.addLayer(layer)
    }
}

// #-end-example-code

struct DDSCircleLayerExampleUIViewControllerRepresentable: UIViewControllerRepresentable {
    typealias UIViewControllerType = DDSCircleLayerExample

    func makeUIViewController(context _: Context) -> DDSCircleLayerExample {
        DDSCircleLayerExample()
    }

    func updateUIViewController(_: DDSCircleLayerExample, context _: Context) {}
}
