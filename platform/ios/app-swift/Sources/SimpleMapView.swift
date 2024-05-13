import MapLibre
import SwiftUI
import UIKit

// #-example-code(SimpleMap)
struct SimpleMap: UIViewRepresentable {
    func makeUIView(context _: Context) -> MLNMapView {
        let mapView = MLNMapView()
        return mapView
    }

    func updateUIView(_: MLNMapView, context _: Context) {}
}

// #-end-example-code
