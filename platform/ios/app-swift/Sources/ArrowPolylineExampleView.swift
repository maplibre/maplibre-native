import CoreLocation
import GluecodiumArrowPolyline
import MapLibre
import SwiftUI
import UIKit

/// Example demonstrating the ArrowPolylineExample cross-platform plugin.
/// Tap on the map to add points, then see the arrow polyline update.
struct ArrowPolylineExampleView: View {
  @State private var plugin = ArrowPolylineExample()
  @State private var coordinates: [LatLng] = []

  var body: some View {
    ZStack(alignment: .bottom) {
      MapView(plugin: plugin, coordinates: $coordinates)

      HStack {
        Button("Clear") {
          coordinates.removeAll()
          plugin.removeArrowPolyline()
        }
        .padding()
        .background(Color.red)
        .foregroundColor(.white)
        .cornerRadius(8)

        Spacer()

        Button("Demo") {
          showDemoArrow()
        }
        .padding()
        .background(Color.blue)
        .foregroundColor(.white)
        .cornerRadius(8)
      }
      .padding()
    }
  }

  private func showDemoArrow() {
    // Demo arrow around SF Bay Area using Gluecodium LatLng directly
    let demoCoords: [LatLng] = [
      LatLng(latitude: 37.8044, longitude: -122.2712),  // Oakland
      LatLng(latitude: 37.7955, longitude: -122.3937),  // Bay Bridge
      LatLng(latitude: 37.7749, longitude: -122.4194),  // Downtown SF
      LatLng(latitude: 37.7599, longitude: -122.4370),  // Twin Peaks
      LatLng(latitude: 37.7339, longitude: -122.4467),  // Glen Park
      LatLng(latitude: 37.7127, longitude: -122.4530),  // Daly City
    ]

    coordinates.append(contentsOf: demoCoords)
    updateArrow()
  }

  private func updateArrow() {
    guard coordinates.count >= 2 else { return }
    let config = ArrowPolylineConfig(
      headLength: 30.0,
      headAngle: 25.0,
      lineColor: "#0066FF",
      lineWidth: 4.0
    )
    plugin.addArrowPolyline(coordinates: coordinates, config: config)
  }

  struct MapView: UIViewRepresentable {
    let plugin: ArrowPolylineExample
    @Binding var coordinates: [LatLng]

    func makeCoordinator() -> Coordinator {
      Coordinator(plugin: plugin, coordinates: $coordinates)
    }

    func makeUIView(context: Context) -> MLNMapView {
      let mapView = MLNMapView(frame: .zero, styleURL: nil, plugins: [plugin.bridge])
      mapView.setCenter(CLLocationCoordinate2D(latitude: 37.7749, longitude: -122.4194),
                        zoomLevel: 10, animated: false)

      let tap = UITapGestureRecognizer(target: context.coordinator, action: #selector(Coordinator.handleTap(_:)))
      mapView.addGestureRecognizer(tap)
      context.coordinator.mapView = mapView

      return mapView
    }

    func updateUIView(_: MLNMapView, context: Context) {}

    class Coordinator: NSObject {
      let plugin: ArrowPolylineExample
      @Binding var coordinates: [LatLng]
      weak var mapView: MLNMapView?

      init(plugin: ArrowPolylineExample, coordinates: Binding<[LatLng]>) {
        self.plugin = plugin
        self._coordinates = coordinates
      }

      @objc func handleTap(_ gesture: UITapGestureRecognizer) {
        guard let mapView else { return }
        let point = gesture.location(in: mapView)
        let coord = mapView.convert(point, toCoordinateFrom: mapView)

        coordinates.append(LatLng(latitude: coord.latitude, longitude: coord.longitude))
        if coordinates.count >= 2 {
          let config = ArrowPolylineConfig(
            headLength: 30.0,
            headAngle: 25.0,
            lineColor: "#0066FF",
            lineWidth: 4.0
          )
          plugin.addArrowPolyline(coordinates: coordinates, config: config)
        }
      }
    }
  }
}
