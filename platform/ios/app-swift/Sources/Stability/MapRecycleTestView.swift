import SwiftUI
import UIKit

struct MapRecycleTestView: View {
    let CYCLE_COUNT = 300

    @Environment(\.dismiss) var dismiss
    @State private var remainingCycles = 300
    @State private var showTopMap = false
    @State private var topMapStarted = false
    @State private var topMapID = UUID()
    @State private var remountTask: Task<Void, Never>?
    @State private var bottomMapHandle = NavigationMapHandle()

    var body: some View {
        VStack(spacing: 0) {
            let style: URL? = nil
            Group {
                if showTopMap {
                    NavigationMapView(
                        onDidFinishLoadingMap: scheduleRemount,
                        cameraSource: bottomMapHandle.map,
                        styleURL: style,
                        loadRoutes: false
                    )
                    .id(topMapID)
                } else {
                    Color.clear
                }
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)

            NavigationMapView(
                onRouteReady: {
                    guard !topMapStarted else { return }
                    topMapStarted = true
                    showTopMap = true
                },
                onCreated: { bottomMapHandle.map = $0 },
                styleURL: style
            )
            .frame(maxWidth: .infinity, maxHeight: .infinity)
        }
        .edgesIgnoringSafeArea(.bottom)
        .onAppear {
            UIApplication.shared.isIdleTimerDisabled = true
            remainingCycles = CYCLE_COUNT
        }
        .onDisappear {
            remountTask?.cancel()
            remountTask = nil
            UIApplication.shared.isIdleTimerDisabled = false
        }
        .overlay(alignment: .topLeading) {
            CountdownBadge(remainingCycles: remainingCycles)
                .padding(16)
        }
    }

    private func scheduleRemount() {
        guard remountTask == nil else { return }

        remountTask = Task { @MainActor in
            do {
                try await Task.sleep(for: .seconds(1))
                showTopMap = false
                remainingCycles -= 1
                if remainingCycles <= 0 {
                    printMemoryUsage()
                    dismiss()
                    return
                }
                try await Task.sleep(for: .seconds(0.1))
                topMapID = UUID()
                showTopMap = true
                remountTask = nil
            } catch {
                remountTask = nil
            }
        }
    }
}
