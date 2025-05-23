import MapLibre
import SwiftUI
import UIKit

class ObserverExampleView: UIViewController, MLNMapViewDelegate {
    var mapView: MLNMapView!

    override func viewDidLoad() {
        super.viewDidLoad()

        mapView = MLNMapView(frame: view.bounds)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]

        view.addSubview(mapView)

        mapView.delegate = self
    }

    func mapViewDidFinishLoadingMap(_: MLNMapView) {
        // #-example-code(enableRenderingStatsView)
        mapView.enableRenderingStatsView(true)
        // #-end-example-code
    }

    // #-example-code(ObserverExampleShaders)
    func mapView(_: MLNMapView, shaderWillCompile id: Int, backend: Int, defines: String) {
        print("A new shader is being compiled - shaderID:\(id), backend type:\(backend), program configuration:\(defines)")
    }

    func mapView(_: MLNMapView, shaderDidCompile id: Int, backend: Int, defines: String) {
        print("A shader has been compiled - shaderID:\(id), backend type:\(backend), program configuration:\(defines)")
    }

    // #-end-example-code

    // #-example-code(ObserverExampleGlyphs)
    func mapView(_: MLNMapView, glyphsWillLoad fontStack: [String], range: NSRange) {
        print("Glyphs are being requested for the font stack \(fontStack), ranging from \(range.location) to \(range.location + range.length)")
    }

    func mapView(_: MLNMapView, glyphsDidLoad fontStack: [String], range: NSRange) {
        print("Glyphs have been loaded for the font stack \(fontStack), ranging from \(range.location) to \(range.location + range.length)")
    }

    // #-end-example-code

    // #-example-code(ObserverExampleTiles)
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

    // #-end-example-code

    // #-example-code(ObserverExampleSprites)
    func mapView(_: MLNMapView, spriteWillLoad id: String, url: String) {
        print("The sprite \(id) has been requested from \(url)")
    }

    func mapView(_: MLNMapView, spriteDidLoad id: String, url: String) {
        print("The sprite \(id) has been loaded from \(url)")
    }
    // #-end-example-code
}

struct ObserverExampleViewUIViewControllerRepresentable: UIViewControllerRepresentable {
    typealias UIViewControllerType = ObserverExampleView

    func makeUIViewController(context _: Context) -> ObserverExampleView {
        ObserverExampleView()
    }

    func updateUIViewController(_: ObserverExampleView, context _: Context) {}
}
