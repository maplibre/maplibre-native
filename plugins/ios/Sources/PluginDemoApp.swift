import MapLibre
import PluginRegistration
import SwiftUI

@main
struct PluginDemoApp: App {
    /// Set when a plugin could not be registered; the demo is useless without them, so it is shown instead of the map.
    private let registrationError: String?

    init() {
        // A style that names an unregistered layer type drops the layer, so register before any map loads a style.
        do {
            try MLNPuckPlugin.register() // the `gltf` layer
            try NgonPlugin.register() // the `ngon` layer
            registrationError = nil
        } catch {
            registrationError = error.localizedDescription
        }
    }

    var body: some Scene {
        WindowGroup {
            if let registrationError {
                Text("Could not register the plugins: \(registrationError)")
                    .padding()
            } else {
                DemoList()
            }
        }
    }
}
