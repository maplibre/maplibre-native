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
        let mapView = MLNMapView(frame: .zero, styleURL: url)
        mapView.attributionButton.isHidden = true
        return mapView
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

/// The bar under a demo's map that holds its controls.
struct ControlBar<Content: View>: View {
    @ViewBuilder var content: Content

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            content
        }
        .padding()
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(.regularMaterial)
    }
}

extension Color {
    /// `#RRGGBB`.
    init(hex: String) {
        var value: UInt64 = 0
        Scanner(string: hex.trimmingCharacters(in: CharacterSet(charactersIn: "#"))).scanHexInt64(&value)
        self.init(
            red: Double((value >> 16) & 0xFF) / 255, green: Double((value >> 8) & 0xFF) / 255,
            blue: Double(value & 0xFF) / 255
        )
    }

    var hex: String {
        var red: CGFloat = 0, green: CGFloat = 0, blue: CGFloat = 0, alpha: CGFloat = 0
        UIColor(self).getRed(&red, green: &green, blue: &blue, alpha: &alpha)
        return String(format: "#%02X%02X%02X", Int((red * 255).rounded()), Int((green * 255).rounded()), Int((blue * 255).rounded()))
    }
}
