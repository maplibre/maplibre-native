import Combine
import MapLibre
import SwiftUI
import UIKit

struct UserMapView: UIViewRepresentable {
    func makeUIView(context _: Context) -> UserMap {
        let map = UserMap()
        map.run()
        return map
    }

    func updateUIView(_: UserMap, context _: Context) {}

    static func dismantleUIView(_ map: UserMap, coordinator _: ()) {
        map.stop()
    }
}

struct NavigationMapView: UIViewRepresentable {
    func makeUIView(context _: Context) -> NavigationMap {
        let map = NavigationMap()
        map.run()
        return map
    }

    func updateUIView(_: NavigationMap, context _: Context) {}

    static func dismantleUIView(_ map: NavigationMap, coordinator _: ()) {
        map.stop()
    }
}

struct LongRunningMapView: View {
    // view lifetime (seconds)
    let DURATION = 72.0 * 60.0 * 60.0

    @Environment(\.dismiss) var dismiss
    @State private var remainingTime: TimeInterval = 72.0 * 60.0 * 60.0

    private let timer = Timer.publish(every: 1.0, on: .main, in: .default).autoconnect()

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
        .onReceive(timer) { _ in
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
