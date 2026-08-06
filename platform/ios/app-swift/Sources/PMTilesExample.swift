import MapLibre
import SwiftUI
import UIKit

// #-example-code(PMTilesStyleURL)
class PMTilesStyleURL: UIViewController {
    override func viewDidLoad() {
        super.viewDidLoad()
        let mapView = MLNMapView(
            frame: view.bounds,
            styleURL: URL(string: "https://demotiles.maplibre.org/pmtiles/raster/style-imagery.json")!
        )
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.setCenter(CLLocationCoordinate2D(latitude: 47.5, longitude: 12.0), zoomLevel: 9, animated: false)
        view.addSubview(mapView)
    }
}

// #-end-example-code

// #-example-code(PMTilesAddSource)
class PMTilesAddSource: UIViewController, MLNMapViewDelegate {
    var mapView: MLNMapView!

    override func viewDidLoad() {
        super.viewDidLoad()
        mapView = MLNMapView(frame: view.bounds)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.delegate = self
        view.addSubview(mapView)
    }

    func mapView(_: MLNMapView, didFinishLoading style: MLNStyle) {
        let overtureURL = URL(string: "pmtiles://https://overturemaps-tiles-us-west-2-beta.s3.amazonaws.com/2026-01-21/places.pmtiles")!
        let overtureSource = MLNVectorTileSource(identifier: "overture-places", configurationURL: overtureURL)
        style.addSource(overtureSource)
        let overtureLayer = MLNCircleStyleLayer(identifier: "overture-layer", source: overtureSource)
        overtureLayer.sourceLayerIdentifier = "place"
        overtureLayer.circleColor = NSExpression(forConstantValue: UIColor(red: 0.96, green: 0.78, blue: 0.0, alpha: 1))
        overtureLayer.circleRadius = NSExpression(forConstantValue: 4)
        overtureLayer.circleOpacity = NSExpression(forConstantValue: 0.6)
        style.addLayer(overtureLayer)

        let foursquareURL = URL(string: "pmtiles://https://oliverwipfli.ch/data/foursquare-os-places-10M-2024-11-20.pmtiles")!
        let foursquareSource = MLNVectorTileSource(identifier: "foursquare-places", configurationURL: foursquareURL)
        style.addSource(foursquareSource)
        let foursquareLayer = MLNCircleStyleLayer(identifier: "foursquare-layer", source: foursquareSource)
        foursquareLayer.sourceLayerIdentifier = "place"
        foursquareLayer.circleColor = NSExpression(forConstantValue: UIColor(red: 0.18, green: 0.85, blue: 1.0, alpha: 1))
        foursquareLayer.circleRadius = NSExpression(forConstantValue: 4)
        foursquareLayer.circleOpacity = NSExpression(forConstantValue: 0.6)
        style.addLayer(foursquareLayer)
    }
}

// #-end-example-code

// #-example-code(PMTilesRasterSource)
class PMTilesRasterSource: UIViewController, MLNMapViewDelegate {
    var mapView: MLNMapView!

    override func viewDidLoad() {
        super.viewDidLoad()
        mapView = MLNMapView(frame: view.bounds)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.delegate = self
        view.addSubview(mapView)
    }

    func mapView(_: MLNMapView, didFinishLoading style: MLNStyle) {
        let url = URL(string: "pmtiles://https://demotiles.maplibre.org/pmtiles/raster/imagery.pmtiles")!
        let source = MLNRasterTileSource(identifier: "imagery", configurationURL: url, tileSize: 256)
        style.addSource(source)
        let layer = MLNRasterStyleLayer(identifier: "imagery", source: source)
        style.addLayer(layer)
    }
}

// #-end-example-code

// #-example-code(PMTilesLocalFile)
class PMTilesLocalFile: UIViewController, MLNMapViewDelegate {
    var mapView: MLNMapView!

    override func viewDidLoad() {
        super.viewDidLoad()
        mapView = MLNMapView(frame: view.bounds)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.delegate = self
        view.addSubview(mapView)
    }

    func mapView(_: MLNMapView, didFinishLoading style: MLNStyle) {
        guard let fileURL = Bundle.main.url(forResource: "world", withExtension: "pmtiles") else { return }
        let url = URL(string: "pmtiles://\(fileURL.absoluteString)")!
        let source = MLNVectorTileSource(identifier: "world", configurationURL: url)
        style.addSource(source)
        let layer = MLNFillStyleLayer(identifier: "countries", source: source)
        layer.sourceLayerIdentifier = "countries"
        layer.fillColor = NSExpression(forConstantValue: UIColor(red: 0.67, green: 0.83, blue: 0.87, alpha: 1))
        style.addLayer(layer)
    }
}

// #-end-example-code

struct PMTilesStyleURLRepresentable: UIViewControllerRepresentable {
    func makeUIViewController(context _: Context) -> PMTilesStyleURL { PMTilesStyleURL() }
    func updateUIViewController(_: PMTilesStyleURL, context _: Context) {}
}

struct PMTilesAddSourceRepresentable: UIViewControllerRepresentable {
    func makeUIViewController(context _: Context) -> PMTilesAddSource { PMTilesAddSource() }
    func updateUIViewController(_: PMTilesAddSource, context _: Context) {}
}

struct PMTilesRasterSourceRepresentable: UIViewControllerRepresentable {
    func makeUIViewController(context _: Context) -> PMTilesRasterSource { PMTilesRasterSource() }
    func updateUIViewController(_: PMTilesRasterSource, context _: Context) {}
}

struct PMTilesLocalFileRepresentable: UIViewControllerRepresentable {
    func makeUIViewController(context _: Context) -> PMTilesLocalFile { PMTilesLocalFile() }
    func updateUIViewController(_: PMTilesLocalFile, context _: Context) {}
}
