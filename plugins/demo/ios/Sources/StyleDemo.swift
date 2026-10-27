import CoreLocation
import MapLibre
import SwiftUI

/// What the plugin generator copies from a plugin's `plugin.json` into the plugin's wrapper (`<id>.demo.json`) when the
/// plugin has a `demo`.
struct PluginDemoMetadata: Decodable {
    struct Camera: Decodable {
        let latitude: Double
        let longitude: Double
        let zoom: Double
        let pitch: Double?
        let bearing: Double?
    }

    let id: String
    let displayName: String
    let description: String
    /// An `https://` style URL, or the name of a style file bundled with the app.
    let style: String
    let camera: Camera?

    var styleURL: URL? {
        style.hasPrefix("https://") ? URL(string: style) : Bundle.main.url(forResource: style, withExtension: nil)
    }

    /// The metadata of every plugin demo bundled with the app, ordered by name.
    static func bundled() -> [PluginDemoMetadata] {
        let urls = Bundle.main.urls(forResourcesWithExtension: "json", subdirectory: nil) ?? []
        return urls
            .filter { $0.lastPathComponent.hasSuffix(".demo.json") }
            .compactMap { try? JSONDecoder().decode(PluginDemoMetadata.self, from: Data(contentsOf: $0)) }
            .sorted { $0.displayName < $1.displayName }
    }
}

/// The demo of a plugin that has no view of its own: the style from its `plugin.json`, looked at through its camera.
struct StyleDemoView: View {
    let metadata: PluginDemoMetadata
    @StateObject private var status = DemoStatus()

    var body: some View {
        ZStack(alignment: .top) {
            StyleMap(metadata: metadata, status: status)
            StatusBanner(status: status)
        }
    }
}

private struct StyleMap: UIViewRepresentable {
    let metadata: PluginDemoMetadata
    let status: DemoStatus

    func makeCoordinator() -> Coordinator { Coordinator(metadata: metadata, status: status) }

    func makeUIView(context: Context) -> MLNMapView {
        let mapView = MLNMapView.demoMap(styleURL: metadata.styleURL)
        mapView.delegate = context.coordinator
        return mapView
    }

    func updateUIView(_: MLNMapView, context _: Context) {}

    @MainActor
    final class Coordinator: NSObject, @preconcurrency MLNMapViewDelegate {
        let metadata: PluginDemoMetadata
        let status: DemoStatus

        init(metadata: PluginDemoMetadata, status: DemoStatus) {
            self.metadata = metadata
            self.status = status
        }

        func mapView(_ mapView: MLNMapView, didFinishLoading _: MLNStyle) {
            guard let camera = metadata.camera else { return }
            mapView.look(
                at: CLLocationCoordinate2D(latitude: camera.latitude, longitude: camera.longitude),
                zoom: camera.zoom, pitch: CGFloat(camera.pitch ?? 0), heading: camera.bearing ?? 0
            )
        }

        func mapViewDidFailLoadingMap(_: MLNMapView, withError error: Error) {
            status.report("\(metadata.id): \(error.localizedDescription)")
        }
    }
}
