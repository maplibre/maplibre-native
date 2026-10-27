import SwiftUI

/// One entry in the list of demos. To show off another plugin, write a view for it and add an entry to `Demo.all`.
struct Demo: Identifiable, Hashable {
    /// Also the value of the `-demo` launch argument that opens the demo directly.
    let id: String
    let title: String
    /// The style layer type the demo draws with.
    let layerType: String
    let summary: String
    let systemImage: String
    let destination: @MainActor () -> AnyView

    init(
        id: String,
        title: String,
        layerType: String,
        summary: String,
        systemImage: String,
        @ViewBuilder destination: @escaping @MainActor () -> some View
    ) {
        self.id = id
        self.title = title
        self.layerType = layerType
        self.summary = summary
        self.systemImage = systemImage
        self.destination = { AnyView(destination()) }
    }

    static func == (lhs: Demo, rhs: Demo) -> Bool { lhs.id == rhs.id }

    func hash(into hasher: inout Hasher) { hasher.combine(id) }

    static let all = [
        Demo(
            id: "models",
            title: "3D models",
            layerType: "gltf",
            summary: "glTF models for vehicles, trees and a user puck, picked per feature.",
            systemImage: "car.fill"
        ) { ModelsDemoView() },
        Demo(
            id: "ngons",
            title: "N-gons",
            layerType: "ngon",
            summary: "Regular polygons drawn by a shader the plugin ships, one per point.",
            systemImage: "hexagon.fill"
        ) { NgonDemoView() },
    ]
}

struct DemoList: View {
    /// `-demo ngons` on the command line (`simctl launch ... -demo ngons`) opens that demo straight away.
    @State private var path = Demo.all.filter { $0.id == UserDefaults.standard.string(forKey: "demo") }

    var body: some View {
        NavigationStack(path: $path) {
            List(Demo.all) { demo in
                NavigationLink(value: demo) {
                    Label {
                        VStack(alignment: .leading, spacing: 2) {
                            Text(demo.title).font(.headline)
                            Text(demo.summary).font(.subheadline).foregroundStyle(.secondary)
                            Text("\"type\": \"\(demo.layerType)\"").font(.caption.monospaced()).foregroundStyle(.tertiary)
                        }
                    } icon: {
                        Image(systemName: demo.systemImage)
                    }
                }
            }
            .navigationTitle("Plugins")
            .navigationDestination(for: Demo.self) { demo in
                demo.destination()
                    .navigationTitle(demo.title)
                    .navigationBarTitleDisplayMode(.inline)
            }
        }
    }
}
