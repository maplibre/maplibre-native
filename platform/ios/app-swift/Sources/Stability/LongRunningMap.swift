import MapLibre
import SwiftUI
import UIKit

struct UserMapView: UIViewRepresentable {
    func makeUIView(context _: Context) -> MLNMapView {
        let view = UserMap()
        view.run()
        return view
    }

    func updateUIView(_: MLNMapView, context _: Context) {}
}

struct NavigationMapView: UIViewRepresentable {
    func makeUIView(context _: Context) -> MLNMapView {
        let view = NavigationMap()
        view.run()
        return view
    }

    func updateUIView(_: MLNMapView, context _: Context) {}
}

struct LongRunningMapView: View {
    // view lifetime (seconds)
    let DURATION = 10.0 * 60.0 * 60.0

    @Environment(\.dismiss) var dismiss

    var body: some View {
        VStack {
            UserMapView()
            NavigationMapView()
        }
        .onReceive(Timer.publish(every: DURATION, on: .current, in: .default).autoconnect()) { _ in
            // TODO: print memory usage
            dismiss()
        }
    }
}
