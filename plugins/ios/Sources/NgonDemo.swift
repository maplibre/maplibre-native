import MapLibre
import SwiftUI

/// The `ngon` layer: regular polygons drawn by a shader the plugin ships, one per point feature. Radius, corners,
/// rotation, color, blur, opacity and stroke are paint properties, so they can come from feature properties or be
/// changed at runtime.
struct NgonDemoView: View {
    @StateObject private var status = DemoStatus()
    @State private var spin = 0.0
    @State private var billboard = true

    var body: some View {
        ZStack(alignment: .top) {
            NgonMap(spin: spin, billboard: billboard, status: status)
            StatusBanner(status: status)
        }
        .safeAreaInset(edge: .bottom, spacing: 0) {
            ControlPanel(
                title: "ngon layer",
                detail: "Corners, color, stroke, blur and opacity come from each feature's properties. "
                    + "The sliders change the layer's paint properties while it is on screen."
            ) {
                HStack {
                    Text("Spin").frame(width: 50, alignment: .leading)
                    Slider(value: $spin, in: 0 ... 360)
                }
                Picker("Alignment", selection: $billboard) {
                    Text("Faces the screen").tag(true)
                    Text("Lies on the map").tag(false)
                }
                .pickerStyle(.segmented)
            }
        }
    }
}

private struct NgonMap: UIViewRepresentable {
    static let center = CLLocationCoordinate2D(latitude: 52.3731, longitude: 4.8922)

    var spin: Double
    var billboard: Bool
    var status: DemoStatus

    func makeCoordinator() -> Coordinator { Coordinator(status: status) }

    func makeUIView(context: Context) -> MLNMapView {
        let mapView = MLNMapView.demoMap(style: "ngons")
        mapView.delegate = context.coordinator
        return mapView
    }

    func updateUIView(_: MLNMapView, context: Context) {
        context.coordinator.apply(spin: spin, billboard: billboard)
    }

    @MainActor
    final class Coordinator: NSObject, @preconcurrency MLNMapViewDelegate {
        let status: DemoStatus
        private var style: MLNStyle?
        private var spin = 0.0
        private var billboard = true

        init(status: DemoStatus) { self.status = status }

        func mapView(_ mapView: MLNMapView, didFinishLoading style: MLNStyle) {
            self.style = style
            mapView.look(at: NgonMap.center, zoom: 15.9, pitch: 40, heading: 0)
            style.addSource(MLNShapeSource(identifier: "shapes", shape: Self.shapes(), options: nil))
            style.add(layer: Self.layer, status: status)
            apply(spin: spin, billboard: billboard)
        }

        func apply(spin: Double, billboard: Bool) {
            self.spin = spin
            self.billboard = billboard
            guard let style else { return }
            // `ngon-rotate` is degrees, added here to each feature's own rotation.
            style.set("ngon-rotate", ["+", ["coalesce", ["get", "rotation"], 0], spin], on: "ngons", status: status)
            let alignment = billboard ? "viewport" : "map"
            style.set("ngon-pitch-alignment", alignment, on: "ngons", status: status)
            style.set("ngon-pitch-scale", alignment, on: "ngons", status: status)
        }

        private static let layer: [String: Any] = [
            "id": "ngons",
            "type": "ngon",
            "source": "shapes",
            "paint": [
                "ngon-radius": ["coalesce", ["get", "radius"], 22],
                "ngon-corners": ["get", "sides"],
                "ngon-color": ["get", "color"],
                "ngon-opacity": ["coalesce", ["get", "opacity"], 1],
                "ngon-blur": ["coalesce", ["get", "blur"], 0],
                "ngon-stroke-width": ["coalesce", ["get", "stroke"], 0],
                "ngon-stroke-color": ["coalesce", ["get", "outline"], "#FFFFFF"],
                "ngon-pitch-alignment": "viewport",
                "ngon-pitch-scale": "viewport",
            ],
        ]

        /// Four rows of six: plain polygons, outlined ones, increasing blur, and increasing rotation with fading opacity.
        private static func shapes() -> MLNShapeCollectionFeature {
            let frame = LocalFrame(origin: NgonMap.center)
            let palette = ["#E76F51", "#F4A261", "#E9C46A", "#2A9D8F", "#457B9D", "#6D597A"]
            var features: [MLNPointFeature] = []

            func add(row: Int, column: Int, _ attributes: [String: Any]) {
                let feature = MLNPointFeature()
                feature.coordinate = frame.coordinate(
                    east: (Double(column) - 2.5) * 50, north: (1.5 - Double(row)) * 75
                )
                feature.attributes = attributes
                features.append(feature)
            }

            for column in 0 ..< 6 {
                let color = palette[column]
                add(row: 0, column: column, ["sides": column + 3, "color": color])
                add(row: 1, column: column, [
                    "sides": column + 3, "color": color, "stroke": 4, "outline": "#264653",
                ])
                add(row: 2, column: column, [
                    "sides": 6, "color": "#457B9D", "radius": 24, "blur": Double(column) / 5,
                ])
                add(row: 3, column: column, [
                    "sides": 5, "color": "#E76F51", "rotation": column * 15, "opacity": 1 - Double(column) * 0.14,
                ])
            }
            return MLNShapeCollectionFeature(shapes: features)
        }
    }
}
