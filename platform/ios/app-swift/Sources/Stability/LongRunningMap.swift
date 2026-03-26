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
    @State private var remainingTime: TimeInterval = 72.0 * 60.0 * 60.0

    var body: some View {
        VStack(spacing: 0) {
            UserMapView()
            NavigationMapView()
        }
        .edgesIgnoringSafeArea(.bottom)
        .onAppear {
            UIApplication.shared.isIdleTimerDisabled = true
            remainingTime = DURATION
        }
        .onDisappear {
            UIApplication.shared.isIdleTimerDisabled = false
        }
        .onReceive(Timer.publish(every: 1.0, on: .current, in: .default).autoconnect()) { _ in
            if remainingTime > 0 {
                remainingTime -= 1.0
            } else {
                printMemoryUsage()
                dismiss()
            }
        }
        .overlay(alignment: .topLeading) {
            CountdownBadge(remainingTime: remainingTime)
                .padding(16)
        }
    }
}
