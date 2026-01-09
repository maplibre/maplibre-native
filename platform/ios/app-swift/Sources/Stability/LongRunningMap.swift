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

struct NavigationMapView: UIViewRepresentable {
    func makeUIView(context _: Context) -> UIView {
        let map = NavigationMap()
        map.run()
        return map
    }

    func updateUIView(_: UIView, context _: Context) {}
}

struct LongRunningMapView: View {
    // view lifetime (seconds)
    let DURATION = 72.0 * 60.0 * 60.0

    @Environment(\.dismiss) var dismiss

    var body: some View {
        VStack {
            UserMapView()
            NavigationMapView()
        }
        .onAppear {
            UIApplication.shared.isIdleTimerDisabled = true
        }
        .onDisappear {
            UIApplication.shared.isIdleTimerDisabled = false
        }
        .onReceive(Timer.publish(every: DURATION, on: .current, in: .default).autoconnect()) { _ in
            printMemoryUsage()
            dismiss()
        }
    }
}
