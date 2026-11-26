import MapLibre
import SwiftUI
import UIKit

/// #-example-code(SimpleMap)
struct SimpleMap: UIViewRepresentable {
    func makeUIView(context _: Context) -> MLNMapView {
        MLNMapView()
    }

    func updateUIView(_: MLNMapView, context _: Context) {}
}

// #-end-example-code
