import CoreLocation
import MapLibre
import SwiftUI

/// A flat east/north frame in meters around an origin. Good enough for a scene a few hundred meters across.
struct LocalFrame {
    let origin: CLLocationCoordinate2D

    func coordinate(east: Double, north: Double) -> CLLocationCoordinate2D {
        let metersPerDegree = 111_320.0
        return CLLocationCoordinate2D(
            latitude: origin.latitude + north / metersPerDegree,
            longitude: origin.longitude + east / (metersPerDegree * cos(origin.latitude * .pi / 180))
        )
    }
}

extension MLNMapView {
    /// A map showing one of the styles bundled with the app (`Styles/<name>.json`).
    static func demoMap(style name: String) -> MLNMapView {
        let url = Bundle.main.url(forResource: name, withExtension: "json")
        return MLNMapView(frame: .zero, styleURL: url)
    }

    /// Looks at `center` from the given zoom level, pitch (degrees from straight down) and compass heading. Needs
    /// the map's final size, so call it once the style has loaded.
    func look(at center: CLLocationCoordinate2D, zoom: Double, pitch: CGFloat, heading: CLLocationDirection) {
        let altitude = MLNAltitudeForZoomLevel(zoom, pitch, center.latitude, bounds.size)
        setCamera(
            MLNMapCamera(lookingAtCenter: center, altitude: altitude, pitch: pitch, heading: heading), animated: false
        )
    }
}

extension MLNStyle {
    /// Sets a property on a plugin layer, reporting failure to `status` instead of throwing.
    @MainActor
    func set(_ property: String, _ value: Any, on layer: String, status: DemoStatus) {
        do {
            try setProperty(property, value: value, forLayerWithIdentifier: layer)
        } catch {
            status.report("\(layer).\(property): \(error.localizedDescription)")
        }
    }

    @MainActor
    func add(layer json: [String: Any], below sibling: String? = nil, status: DemoStatus) {
        do {
            try addLayer(withJSON: json, belowLayerWithIdentifier: sibling)
        } catch {
            status.report("\(json["id"] ?? "layer"): \(error.localizedDescription)")
        }
    }
}

/// The first thing that went wrong in a scene, shown over the map so a broken plugin layer isn't a silent blank.
@MainActor
final class DemoStatus: ObservableObject {
    @Published private(set) var message: String?

    func report(_ message: String) {
        print("PluginDemo: \(message)")
        self.message = self.message ?? message
    }
}

struct StatusBanner: View {
    @ObservedObject var status: DemoStatus

    var body: some View {
        if let message = status.message {
            Text(message)
                .font(.footnote)
                .foregroundStyle(.white)
                .padding(8)
                .frame(maxWidth: .infinity, alignment: .leading)
                .background(.red.opacity(0.85))
        }
    }
}

/// The caption and controls under a scene's map.
struct ControlPanel<Content: View>: View {
    let title: String
    let detail: String
    @ViewBuilder var content: Content

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text(title).font(.headline)
            Text(detail).font(.footnote).foregroundStyle(.secondary)
            content
        }
        .padding()
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(.regularMaterial)
    }
}
