import MapLibre
import SwiftUI
import UIKit

// #-example-code(SimpleMapView)
struct SimpleMapView: UIViewRepresentable {
    func makeUIView(context _: Context) -> MLNMapView {
        let mapView = MLNMapView()
        return mapView
    }

    func updateUIView(_: MLNMapView, context _: Context) {}
}

// #-end-example-code

struct ContentView: View {
    var body: some View {
        SimpleMapView().edgesIgnoringSafeArea(.all)
    }
}
