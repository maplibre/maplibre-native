import PluginRegistry
import SwiftUI

/// One entry in the list of demos. To show a plugin, write its view and add an entry to `Demo.all`.
struct Demo: Identifiable, Hashable {
    /// Also the value of the `-demo` launch argument that opens the demo directly.
    let id: String
    /// The directory name of the plugin in `plugins/plugins`. The demo registers the plugin when it opens.
    let pluginID: String
    let title: String
    /// The style layer type the demo draws with, if it is shown.
    let layerType: String?
    let summary: String
    let systemImage: String
    let destination: @MainActor () -> AnyView

    init(
        id: String,
        pluginID: String,
        title: String,
        layerType: String? = nil,
        summary: String,
        systemImage: String,
        @ViewBuilder destination: @escaping @MainActor () -> some View
    ) {
        self.id = id
        self.pluginID = pluginID
        self.title = title
        self.layerType = layerType
        self.summary = summary
        self.systemImage = systemImage
        self.destination = { AnyView(RegisteredDemo(pluginID: pluginID, content: destination)) }
    }

    static func == (lhs: Demo, rhs: Demo) -> Bool { lhs.id == rhs.id }

    func hash(into hasher: inout Hasher) { hasher.combine(id) }

    static let all = [
        Demo(
            id: "ngons",
            pluginID: "ngon-layer",
            title: "N-gons",
            layerType: "ngon",
            summary: "A regular polygon drawn by a shader the plugin ships, with its properties to edit or randomize.",
            systemImage: "hexagon.fill"
        ) { NgonDemoView() },
    ]
}

/// Registers a demo's plugin before the demo creates its map: a style that names an unregistered layer type drops the
/// layer. A plugin registers for the rest of the process, so a plugin that replaces a built-in layer keeps replacing
/// it in the demos opened afterwards.
private struct RegisteredDemo<Content: View>: View {
    let pluginID: String
    let content: () -> Content

    var body: some View {
        if let failure = Self.register(pluginID) {
            Text("Could not register the \(pluginID) plugin: \(failure)")
                .padding()
        } else {
            content()
        }
    }

    private static func register(_ pluginID: String) -> String? {
        do {
            try PluginRegistry.registerPlugin(identifier: pluginID)
            return nil
        } catch {
            return error.localizedDescription
        }
    }
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
                            if let layerType = demo.layerType {
                                Text("\"type\": \"\(layerType)\"").font(.caption.monospaced()).foregroundStyle(.tertiary)
                            }
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
