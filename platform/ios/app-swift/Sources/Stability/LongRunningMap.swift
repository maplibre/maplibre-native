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
    var onDidFinishLoadingMap: (() -> Void)?
    var onRouteReady: (() -> Void)?
    var cameraSource: MLNMapView?
    var onCreated: ((NavigationMap) -> Void)?
    var styleURL: URL?
    var loadRoutes: Bool = true
    var trimRoute: Bool = false
    var onRenderStats: ((_ framesPerSecond: Double, _ zoomLevel: Double, _ routePointCount: Int) -> Void)?
    var onControlStateChange: ((NavigationMapControlState) -> Void)?

    func makeUIView(context _: Context) -> NavigationMap {
        let map = NavigationMap(cameraSource: cameraSource, styleURL: styleURL, loadRoutes: loadRoutes, trimRoute: trimRoute)
        map.onDidFinishLoadingMap = onDidFinishLoadingMap
        map.onRouteReady = onRouteReady
        map.onRenderStats = onRenderStats
        map.onControlStateChange = onControlStateChange
        map.run()
        onCreated?(map)
        return map
    }

    func updateUIView(_ map: NavigationMap, context _: Context) {
        map.onDidFinishLoadingMap = onDidFinishLoadingMap
        map.onRouteReady = onRouteReady
        map.onRenderStats = onRenderStats
        map.onControlStateChange = onControlStateChange
    }

    static func dismantleUIView(_ map: NavigationMap, coordinator _: ()) {
        map.stop()
    }
}

struct LongRunningMapView: View {
    // view lifetime (seconds)
    let DURATION = 72.0 * 60.0 * 60.0
    var showUserMap = true
    var trimRoute = false

    @Environment(\.dismiss) var dismiss
    @State private var remainingTime: TimeInterval = 72.0 * 60.0 * 60.0
    @State private var framesPerSecond = 0.0
    @State private var zoomLevel = 0.0
    @State private var routePointCount = 0
    @State private var controlState = NavigationMapControlState()
    @State private var mapHandle = NavigationMapHandle()

    private let timer = Timer.publish(every: 1.0, on: .main, in: .default).autoconnect()

    var body: some View {
        Group {
            if showUserMap {
                VStack(spacing: 0) {
                    UserMapView()
                    navigationMap
                }
            } else {
                navigationMap
            }
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
        .overlay(alignment: .top) {
            HStack(alignment: .top) {
                CountdownBadge(remainingTime: remainingTime)
                Spacer(minLength: 8)
                if trimRoute {
                    MapStatsBadge(
                        framesPerSecond: framesPerSecond,
                        zoomLevel: zoomLevel,
                        routePointCount: routePointCount
                    )
                }
            }
            .padding(16)
            .allowsHitTesting(false)
        }
        .overlay(alignment: .leading) {
            if trimRoute {
                routeControlButtons
                    .padding(.leading, 12)
            }
        }
    }

    private var routeControlButtons: some View {
        VStack(alignment: .leading, spacing: 8) {
            routeControlButton("Next route") {
                mapHandle.map?.skipToNextRoute()
            }
            if controlState.canCycleStyle {
                routeControlButton("Next style") {
                    mapHandle.map?.cycleToNextStyle()
                }
            }
            routeControlButton(
                NavigationMap.trackingModeTitle(controlState.userTrackingMode),
                emphasized: controlState.userTrackingMode != .none
            ) {
                mapHandle.map?.cycleTrackingMode()
            }
            routeControlButton(controlState.trimToVisibleExtent ? "Visible Extent" : "Full Route", emphasized: controlState.trimToVisibleExtent) {
                mapHandle.map?.toggleVisibleExtentTrim()
            }
        }
    }

    private func routeControlButton(
        _ title: String,
        emphasized: Bool = true,
        action: @escaping () -> Void
    ) -> some View {
        Button(action: action) {
            Text(title)
                .font(.system(size: 15, weight: .semibold))
                .foregroundColor(.white)
                .padding(.horizontal, 12)
                .padding(.vertical, 10)
                .background(MapLibreColors.primary.opacity(emphasized ? 0.92 : 0.45))
                .cornerRadius(12)
        }
        .buttonStyle(.plain)
    }

    private var navigationMap: some View {
        NavigationMapView(
            onCreated: { mapHandle.map = $0 },
            trimRoute: trimRoute,
            onRenderStats: trimRoute ? { fps, zoom, points in
                framesPerSecond = fps
                zoomLevel = zoom
                routePointCount = points
            } : nil,
            onControlStateChange: trimRoute ? { controlState = $0 } : nil
        )
    }
}
