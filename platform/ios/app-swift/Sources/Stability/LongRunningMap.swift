import MapLibre
import SwiftUI
import UIKit

struct UserMapView: UIViewRepresentable {
    func makeUIView(context _: Context) -> UIView {
        let map = UserMap()
        map.run()
        return map
    }

    func updateUIView(_: UIView, context _: Context) {}
}

struct StandardNavigationMapView: UIViewControllerRepresentable {
    func makeUIViewController(context _: Context) -> UIViewController {
        let map = StandardNavigationMap()
        map.run()
        return map
    }

    func updateUIViewController(_: UIViewController, context _: Context) {}
}

struct SimpleNavigationMapView: UIViewRepresentable {
    func makeUIView(context _: Context) -> UIView {
        let map = SimpleNavigationMap()
        map.run()
        return map
    }

    func updateUIView(_: UIView, context _: Context) {}
}

struct LongRunningMapView: View {
    // view lifetime (seconds)
    let DURATION = 10.0 * 60.0 * 60.0
    // use the built-in navigation map UI provided by the plugin
    let USE_STANDARD_NAVIGATION = false

    @Environment(\.dismiss) var dismiss

    var body: some View {
        VStack {
            UserMapView()

            if USE_STANDARD_NAVIGATION {
                StandardNavigationMapView()
            } else {
                SimpleNavigationMapView()
            }
        }
        .onReceive(Timer.publish(every: DURATION, on: .current, in: .default).autoconnect()) { _ in
            printMemoryUsage()
            dismiss()
        }
    }
}
