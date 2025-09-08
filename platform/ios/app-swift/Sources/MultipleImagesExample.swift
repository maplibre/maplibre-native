
import MapLibre
import SwiftUI
import UIKit

// #-example-code(MultipleImagesExample)
class MultipleImagesExample: UIViewController, MLNMapViewDelegate {
    var mapView: MLNMapView!

    override func viewDidLoad() {
        super.viewDidLoad()

        mapView = MLNMapView(frame: view.bounds, styleURL: AMERICANA_STYLE)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.tintColor = .darkGray

        let glacierPoint = CLLocationCoordinate2D(latitude: 37.72836462778902, longitude: -119.57352616838511)
        mapView.setCenter(glacierPoint, animated: false)
        mapView.zoomLevel = 12

        mapView.delegate = self
        view.addSubview(mapView)
    }

    // Wait until the style is loaded before modifying the map style.
    func mapView(_: MLNMapView, didFinishLoading style: MLNStyle) {
        let source = MLNVectorTileSource(identifier: "pois-nps", configurationURL: URL(string: "mbtiles://\(Bundle.main.bundlePath)/pois-nps.mbtiles")!)

        style.addSource(source)

        let imagesToAdd = [
            ["nps-restrooms", "restrooms"],
            ["nps-trailhead", "trailhead"],
            ["nps-viewpoint", "viewpoint"],
        ]

        for imageInfo in imagesToAdd {
            if let imageName = imageInfo.first, let imageKey = imageInfo.last {
                if let image = UIImage(named: imageName) {
                    style.setImage(image, forName: imageKey)
                } else {
                    print("Failed to load \(imageName)")
                    return
                }
            }
        }

        let imageLayer = MLNSymbolStyleLayer(identifier: "npc-poi-images", source: source)
        imageLayer.sourceLayerIdentifier = "pois"
        imageLayer.iconImageName = NSExpression(mglJSONObject: [
            "match", ["get", "POITYPE"],
            "Restroom", "restrooms",
            "Trailhead", "trailhead",
            "Viewpoint", "viewpoint",
            "",
        ])

        style.addLayer(imageLayer)
    }
}

// #-end-example-code

struct MultipleImagesExampleUIViewControllerRepresentable: UIViewControllerRepresentable {
    typealias UIViewControllerType = MultipleImagesExample

    func makeUIViewController(context _: Context) -> MultipleImagesExample {
        MultipleImagesExample()
    }

    func updateUIViewController(_: MultipleImagesExample, context _: Context) {}
}
