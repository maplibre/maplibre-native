import MapLibre
import MLNBasicPluginExample
import SwiftUI
import UIKit

/// Example demonstrating the use of a cross-platform plugin with MLNMapView.
struct BasicPluginExampleView: View {
  // Plugins must outlive the mapview. A mapview may re-use a plugin and a plugin may be used in multiple mapviews.
  // Plugin authors should ensure these usecases are supported and ensure proper cleanup via onLoad and onUnload.
  @State private var plugin = MLNBasicPluginExample()

  var body: some View {
    ZStack(alignment: .bottomTrailing) {
      MapView(plugin: plugin)

      Button("Show San Francisco") {
        plugin.showSanFrancisco()
      }
      .padding()
      .background(Color.blue)
      .foregroundColor(.white)
      .cornerRadius(8)
      .padding()
    }
  }

  struct MapView: UIViewRepresentable {
    let plugin: MLNBasicPluginExample

    func makeUIView(context: Context) -> MLNMapView {
      MLNMapView(frame: .zero, styleURL: nil, plugins: [plugin])
    }

    func updateUIView(_: MLNMapView, context: Context) {}
  }
}
