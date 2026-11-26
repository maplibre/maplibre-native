import CoreLocation
import MapLibre
import SwiftUI

/// The `ngon` layer: a regular polygon drawn by a shader the plugin ships, for each point feature. Radius, corners,
/// rotation, color, blur, opacity and stroke are paint properties read from the feature's properties, so changing one
/// of them only edits the feature's data.
struct NgonDemoView: View {
    private enum Tab: String, CaseIterable {
        case map = "Map"
        case json = "JSON"
    }

    @StateObject private var status = DemoStatus()
    @StateObject private var model = NgonModel()
    @State private var tab = Tab.map

    var body: some View {
        VStack(spacing: 0) {
            Picker("View", selection: $tab) {
                ForEach(Tab.allCases, id: \.self) { Text($0.rawValue).tag($0) }
            }
            .pickerStyle(.segmented)
            .padding(.horizontal)
            .padding(.vertical, 8)

            // The map stays in the hierarchy under the JSON, so switching tabs doesn't reload the style.
            ZStack(alignment: .top) {
                NgonMap(model: model, status: status)
                StatusBanner(status: status)
                if tab == .json {
                    NgonJSONView(model: model)
                }
            }
            .safeAreaInset(edge: .bottom, spacing: 0) {
                if tab == .map {
                    ControlBar { NgonControls(ngon: $model.ngon) }
                }
            }
        }
        .toolbar {
            ToolbarItem(placement: .primaryAction) {
                Button(action: model.randomize) {
                    Label("Randomize", systemImage: "dice")
                }
            }
        }
    }
}

// MARK: Model

/// The n-gon's feature properties, which the layer's paint expressions read.
struct Ngon: Equatable {
    var sides = 5
    var color = "#E76F51"
    var radius = 60.0 // points
    var rotation = 0.0 // degrees
    var opacity = 1.0
    var blur = 0.0
    var stroke = 0.0 // outline width in points
    var outline = "#264653"

    var properties: [String: Any] {
        [
            "sides": sides, "color": color, "radius": radius, "rotation": rotation, "opacity": opacity,
            "blur": blur, "stroke": stroke, "outline": outline,
        ]
    }
}

@MainActor
final class NgonModel: ObservableObject {
    @Published var ngon = Ngon()

    func randomize() {
        func rounded(_ value: Double) -> Double { (value * 100).rounded() / 100 }
        func color() -> String {
            Color(
                hue: .random(in: 0 ... 1), saturation: .random(in: 0.45 ... 0.9), brightness: .random(in: 0.6 ... 0.95)
            ).hex
        }
        ngon = Ngon(
            sides: .random(in: 3 ... 12),
            color: color(),
            radius: rounded(.random(in: 24 ... 80)),
            rotation: rounded(.random(in: 0 ..< 360)),
            opacity: rounded(.random(in: 0.5 ... 1)),
            blur: .random() ? 0 : rounded(.random(in: 0.1 ... 0.6)),
            stroke: .random() ? 0 : rounded(.random(in: 1 ... 8)),
            outline: color()
        )
    }
}

// MARK: Style

enum NgonStyle {
    /// The layer, from `ngons.json`. Every paint property reads a feature property.
    static let layer = bundledStyleLayer("ngons", in: "ngons")
}

func prettyJSON(_ object: Any) -> String {
    guard JSONSerialization.isValidJSONObject(object),
          let data = try? JSONSerialization.data(
              withJSONObject: object, options: [.prettyPrinted, .sortedKeys, .withoutEscapingSlashes]
          )
    else { return "" }
    return String(decoding: data, as: UTF8.self)
}

// MARK: Controls and JSON

private struct NgonControls: View {
    @Binding var ngon: Ngon

    var body: some View {
        VStack(spacing: 6) {
            HStack {
                Stepper("Corners  \(ngon.sides)", value: $ngon.sides, in: 3 ... 12)
                ColorPicker("Color", selection: color(\.color)).labelsHidden()
            }
            slider("Radius", $ngon.radius, 8 ... 100)
            slider("Rotation", $ngon.rotation, 0 ... 360)
            slider("Opacity", $ngon.opacity, 0.1 ... 1)
            slider("Blur", $ngon.blur, 0 ... 1)
            HStack {
                slider("Outline", $ngon.stroke, 0 ... 8)
                ColorPicker("Outline color", selection: color(\.outline)).labelsHidden()
            }
        }
        .font(.subheadline)
    }

    private func slider(_ title: String, _ value: Binding<Double>, _ range: ClosedRange<Double>) -> some View {
        HStack {
            Text(title).frame(width: 70, alignment: .leading)
            Slider(value: value, in: range)
        }
    }

    private func color(_ keyPath: WritableKeyPath<Ngon, String>) -> Binding<Color> {
        Binding(
            get: { Color(hex: ngon[keyPath: keyPath]) },
            set: { ngon[keyPath: keyPath] = $0.hex }
        )
    }
}

/// The style of the `ngon` layer, and the properties of its feature, as JSON.
private struct NgonJSONView: View {
    @ObservedObject var model: NgonModel

    var body: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 20) {
                section("Layer", prettyJSON(NgonStyle.layer))
                section("Feature properties", prettyJSON(model.ngon.properties))
            }
            .padding()
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
        .background(Color(.systemBackground))
    }

    private func section(_ title: String, _ text: String) -> some View {
        VStack(alignment: .leading, spacing: 6) {
            Text(title).font(.headline)
            Text(text)
                .font(.system(.footnote, design: .monospaced))
                .textSelection(.enabled)
                .frame(maxWidth: .infinity, alignment: .leading)
        }
    }
}

// MARK: Map

private struct NgonMap: UIViewRepresentable {
    static let center = CLLocationCoordinate2D(latitude: 52.3731, longitude: 4.8922)

    @ObservedObject var model: NgonModel
    var status: DemoStatus

    func makeCoordinator() -> Coordinator { Coordinator(model: model, status: status) }

    func makeUIView(context: Context) -> MLNMapView {
        let mapView = MLNMapView.demoMap(style: "ngons")
        mapView.delegate = context.coordinator
        return mapView
    }

    func updateUIView(_: MLNMapView, context: Context) {
        context.coordinator.sync()
    }

    @MainActor
    final class Coordinator: NSObject, @preconcurrency MLNMapViewDelegate {
        let model: NgonModel
        let status: DemoStatus
        private var shapes: MLNShapeSource?
        private var drawn: Ngon?

        init(model: NgonModel, status: DemoStatus) {
            self.model = model
            self.status = status
        }

        func mapView(_ mapView: MLNMapView, didFinishLoading style: MLNStyle) {
            mapView.look(at: NgonMap.center, zoom: 15.9, pitch: 40, heading: 0)
            shapes = style.demoSource("shapes", status: status)
            drawn = nil
            sync()
        }

        func mapViewDidFailLoadingMap(_: MLNMapView, withError error: Error) {
            status.report("ngons: \(error.localizedDescription)")
        }

        /// Draws the model, when it has changed.
        func sync() {
            guard let shapes, drawn != model.ngon else { return }
            shapes.shape = Self.shape(model.ngon)
            drawn = model.ngon
        }

        private static func shape(_ ngon: Ngon) -> MLNPointFeature {
            let feature = MLNPointFeature()
            feature.coordinate = NgonMap.center
            feature.attributes = ngon.properties
            return feature
        }
    }
}
