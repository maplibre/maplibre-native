import MapLibre
import UIKit

private class NavigationConfig {
    var RANDOM = RandomNumberGeneratorWithSeed(seed: stabilityRandomSeed())

    let STYLES = [
        AMERICANA_STYLE,
        OPENFREEMAP_LIBERTY_STYLE,
        OPENFREEMAP_BRIGHT_STYLE,
        PROTOMAPS_LIGHT_STYLE,
        PROTOMAPS_DARK_STYLE,
        PROTOMAPS_GRAYSCALE_STYLE,
        PROTOMAPS_WHITE_STYLE,
        PROTOMAPS_BLACK_STYLE,
    ]

    func random(in range: ClosedRange<Int>) -> Int {
        Int.random(in: range, using: &RANDOM)
    }

    func random(in range: ClosedRange<Double>) -> Double {
        Double.random(in: range, using: &RANDOM)
    }

    func randomAltitude() -> Double {
        random(in: 800.0 ... 2000.0)
    }

    func randomTilt() -> Double {
        random(in: 30.0 ... 60.0)
    }

    func randomWaitTime() -> TimeInterval {
        random(in: 5000 ... 10000) / 1000.0
    }

    func randomSpeed() -> Double {
        random(in: 0.7 ... 2.0)
    }

    private var routeDirectory: URL {
        URL(fileURLWithPath: Bundle.main.resourcePath! + "/Routes/")
    }

    func getRouteFiles() throws -> [URL] {
        try FileManager.default.contentsOfDirectory(at: routeDirectory, includingPropertiesForKeys: nil)
            .filter { $0.pathExtension.lowercased() == "json" }
    }

    func selectRandomRouteFile() -> URL? {
        do {
            return try getRouteFiles().randomElement(using: &RANDOM)
        } catch {
            print("NavigationMap: \(error)")
            return nil
        }
    }

    func getRouteJson(filename: URL) -> [String: Any]? {
        do {
            let jsonStr = try String(contentsOf: filename, encoding: .utf8)
            return try JSONSerialization.jsonObject(with: jsonStr.data(using: .utf8)!, options: []) as? [String: Any]
        } catch {
            print("NavigationMap: \(error)")
            return nil
        }
    }
}

struct NavigationMapControlState: Equatable {
    var canCycleStyle = false
    var userTrackingMode: MLNUserTrackingMode = .none
    var trimToVisibleExtent = true
}

class NavigationMap: MLNMapView, MLNMapViewDelegate, NavigationLocationManagerDelegate {
    fileprivate let config = NavigationConfig()
    private var route: NavigationRoute?
    private var styleGate: Gate?
    private var task: Task<Void, Never>?
    private var stopped = false
    private var notifyOnMapLoad = false
    private var styleReady = false
    private var waitedForInitialStyle = false
    private var mapDidFinishLoading = false
    private var pendingInitialCamera: MLNMapCamera?
    private var pendingInitialZoom: Double?
    var onDidFinishLoadingMap: (() -> Void)?
    var onRouteReady: (() -> Void)?
    var onRenderStats: ((_ framesPerSecond: Double, _ zoomLevel: Double, _ routePointCount: Int) -> Void)?
    var onControlStateChange: ((NavigationMapControlState) -> Void)?
    private var frameTimes: [CFTimeInterval] = []
    private var lastStatsPublish: CFTimeInterval = 0
    private let specifiedStyleURL: URL?
    private let loadRoutes: Bool
    private let trimRoute: Bool
    private var trimToVisibleExtent = true
    private var styleCycleIndex = 0
    private var styleTask: Task<Void, Never>?
    private var frameTimeGraphView: FrameTimeGraphView?
    private static let trimRouteDefaultUpdateInterval: TimeInterval = 1.0

    init(cameraSource: MLNMapView? = nil, styleURL: URL? = nil, loadRoutes: Bool = true, trimRoute: Bool = false) {
        specifiedStyleURL = styleURL
        self.loadRoutes = loadRoutes
        self.trimRoute = trimRoute

        let effectiveStyle = styleURL ?? config.STYLES.randomElement(using: &config.RANDOM)!!
        super.init(frame: CGRect(), styleURL: effectiveStyle)

        showTransientMessage("Loading \(effectiveStyle)", autoHideAfter: 2.0)

        if let cameraSource {
            pendingInitialCamera = cameraSource.camera
            pendingInitialZoom = cameraSource.zoomLevel + 2
            applyPendingInitialCamera()
        }
    }

    override func layoutSubviews() {
        super.layoutSubviews()

        if pendingInitialCamera != nil, bounds.width > 0, bounds.height > 0 {
            applyPendingInitialCamera()
            pendingInitialCamera = nil
            pendingInitialZoom = nil
        }

        if let graph = frameTimeGraphView {
            let height = min(100, bounds.height * 0.22)
            graph.frame = CGRect(x: 0, y: bounds.height - height, width: bounds.width, height: height)
            bringSubviewToFront(graph)
        }

        layoutTransientMessage()
        bringTransientMessageToFrontIfNeeded()
    }

    private func applyPendingInitialCamera() {
        guard let camera = pendingInitialCamera else { return }

        setCenter(camera.centerCoordinate,
                  zoomLevel: pendingInitialZoom ?? zoomLevel,
                  direction: 0,
                  animated: false)
    }

    @available(*, unavailable)
    @MainActor required init?(coder _: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    @MainActor func load(style: URL) async {
        if style != styleURL {
            showTransientMessage("Loading \(style)")
            styleReady = false
            mapDidFinishLoading = false
            notifyOnMapLoad = true
            styleURL = style
        }
        await waitForStyle()
    }

    @MainActor private func waitForStyle() async {
        notifyOnMapLoad = true
        if mapDidFinishLoading {
            notifyOnMapLoad = false
            onDidFinishLoadingMap?()
        }
        if styleReady { return }

        let gate = Gate()
        styleGate = gate
        if styleReady {
            gate.open()
        }
        await gate.wait()
        styleGate = nil
    }

    @objc(mapView:didFinishLoadingStyle:) func mapView(_: MLNMapView, didFinishLoading _: MLNStyle) {
        styleReady = true
        styleGate?.open()
    }

    func mapViewDidFailLoadingMap(_: MLNMapView, withError error: Error) {
        // a failed style load never reports didFinishLoading
        print("NavigationMap: \(error)")
        styleReady = true
        styleGate?.open()
    }

    func mapViewDidFinishLoadingMap(_: MLNMapView) {
        mapDidFinishLoading = true
        guard notifyOnMapLoad else { return }
        notifyOnMapLoad = false
        onDidFinishLoadingMap?()
    }

    func run() {
        delegate = self
        addRestoreTrackingTapGesture()
        if loadRoutes {
            addRouteControlGestures()
        }

        if trimRoute {
            installFrameTimeGraph()
            dynamicNavigationCameraAnimationDuration = true
            publishControlState()
        }

        if loadRoutes {
            startNewRoute(nextStyle: true, delay: false)
        } else {
            task = Task { @MainActor [weak self] in
                await self?.waitForStyle()
            }
        }
    }

    /// Advance through every tracking mode, including off.
    func cycleTrackingMode() {
        let next: MLNUserTrackingMode
        switch userTrackingMode {
        case .none:
            next = .follow
        case .follow:
            next = .followWithHeading
        case .followWithHeading:
            next = .followWithCourse
        case .followWithCourse:
            next = .none
        @unknown default:
            next = .follow
        }
        userTrackingMode = next
        showTransientMessage(Self.trackingModeMessage(next), autoHideAfter: 2)
    }

    static func trackingModeTitle(_ mode: MLNUserTrackingMode) -> String {
        switch mode {
        case .none:
            return "Tracking Off"
        case .follow:
            return "Follow"
        case .followWithHeading:
            return "Heading"
        case .followWithCourse:
            return "Course"
        @unknown default:
            return "Track"
        }
    }

    private static func trackingModeMessage(_ mode: MLNUserTrackingMode) -> String {
        switch mode {
        case .none:
            return "Tracking off"
        case .follow:
            return "Following location"
        case .followWithHeading:
            return "Following heading"
        case .followWithCourse:
            return "Following course"
        @unknown default:
            return "Tracking"
        }
    }

    func mapView(_: MLNMapView, didChange _: MLNUserTrackingMode, animated _: Bool) {
        publishControlState()
    }

    func publishControlState() {
        let state = NavigationMapControlState(
            canCycleStyle: specifiedStyleURL == nil,
            userTrackingMode: userTrackingMode,
            trimToVisibleExtent: trimToVisibleExtent
        )
        let callback = onControlStateChange
        // Defer so that the write is not inside the SwiftUI update.
        DispatchQueue.main.async {
            callback?(state)
        }
    }

    func cycleToNextStyle() {
        guard specifiedStyleURL == nil, !stopped else { return }
        guard let next = pendingStyle() else { return }

        styleTask?.cancel()
        let routeTask = task
        styleTask = Task { @MainActor [weak self] in
            guard let self else { return }
            // A route load owns the style gate. Wait it out, then swap style on the line it installed.
            if route == nil, let routeTask {
                await routeTask.value
            }
            guard !Task.isCancelled, !stopped else { return }
            await load(style: next.url)
            guard !Task.isCancelled, !stopped else { return }
            styleCycleIndex = next.index
            route?.reinstallDisplayedLine()
        }
    }

    /// The next style in the cycle. The index is stored only after `load(style:)` sticks,
    /// so a cancelled tap does not skip a style.
    private func pendingStyle() -> (url: URL, index: Int)? {
        let styles = config.STYLES.compactMap { $0 }
        guard !styles.isEmpty else { return nil }
        let index: Int
        if let current = styleURL, let currentIndex = styles.firstIndex(of: current) {
            index = (currentIndex + 1) % styles.count
        } else {
            index = (styleCycleIndex + 1) % styles.count
        }
        return (styles[index], index)
    }

    func toggleVisibleExtentTrim() {
        trimToVisibleExtent.toggle()
        route?.trimToVisibleExtent = trimToVisibleExtent
        route?.reapplyDisplayedGeometry()
        publishControlState()
        showTransientMessage(
            trimToVisibleExtent ? "Trimming route to the visible extent" : "Showing the full remaining route",
            autoHideAfter: 2
        )
    }

    private func installFrameTimeGraph() {
        guard frameTimeGraphView == nil else { return }

        let graph = FrameTimeGraphView()
        graph.backgroundColor = UIColor.black.withAlphaComponent(0.25)
        addSubview(graph)
        frameTimeGraphView = graph
    }

    func mapViewDidFinishRenderingFrame(
        _: MLNMapView,
        fullyRendered _: Bool,
        renderingStats: MLNRenderingStats
    ) {
        guard !stopped else { return }
        frameTimeGraphView?.addFrameDuration(renderingStats.encodingTime)
        publishRenderStatsIfNeeded()
        if trimRoute, let route {
            // A step that is still running, including the frame that finishes it,
            // already trims to the current extent. Refresh only while idle, so
            // that finishing frame is not uploaded twice.
            let stepActive = route.trimAnimationActive
            route.tickDisplayedGeometry()
            if !stepActive {
                route.refreshVisibleExtentTrimIfNeeded()
            }
        }
    }

    /// FPS over the last second, published a few times a second with the current zoom.
    private func publishRenderStatsIfNeeded() {
        guard let onRenderStats else { return }

        let now = CACurrentMediaTime()
        frameTimes.append(now)
        let window: CFTimeInterval = 1
        if let oldest = frameTimes.first, now - oldest > window {
            frameTimes.removeAll { now - $0 > window }
        }

        guard frameTimes.count >= 2, now - lastStatsPublish >= 0.25 else { return }
        let elapsed = frameTimes[frameTimes.count - 1] - frameTimes[0]
        guard elapsed > 0 else { return }

        lastStatsPublish = now
        let framesPerSecond = Double(frameTimes.count - 1) / elapsed
        onRenderStats(framesPerSecond, zoomLevel, route?.displayedPointCount ?? 0)
    }

    func stop() {
        guard !stopped else { return }

        stopped = true
        task?.cancel()
        task = nil

        // release a pending style load so the task can observe the cancellation
        // and stop holding on to this map view
        styleGate?.open()

        (locationManager as? NavigationLocationManager)?.navigationDelegate = nil

        // drops the simulated location manager, stopping its perform-based
        // update chain (which otherwise keeps the map view alive)
        let route = route
        self.route = nil
        route?.unload()

        notifyOnMapLoad = false
        onDidFinishLoadingMap = nil
        onRouteReady = nil
        onRenderStats = nil
        onControlStateChange = nil
        frameTimes.removeAll()

        frameTimeGraphView?.removeFromSuperview()
        frameTimeGraphView = nil

        styleTask?.cancel()
        styleTask = nil

        removeTransientMessage()
    }

    deinit {
        task?.cancel()
    }

    func startNewRoute(nextStyle: Bool, delay: Bool = false) {
        guard !stopped else { return }

        styleTask?.cancel()
        styleTask = nil
        task?.cancel()
        task = Task { @MainActor [weak self] in
            do {
                if delay {
                    guard let waitTime = self?.config.randomWaitTime() else { return }
                    self?.showTransientMessage("Waiting \(String(format: "%.1f", waitTime))s …")
                    try await Task.sleep(for: .seconds(waitTime))
                }

                guard let self, !Task.isCancelled else { return }

                // Release the previous route before loading the next one.
                // Its deinit stops location updates and nuls `locationManager`;
                // if that runs after the new route starts, the new simulation dies.
                let previousRoute = route
                route = nil
                previousRoute?.unload()

                if specifiedStyleURL == nil, waitedForInitialStyle, nextStyle {
                    await load(style: config.STYLES.randomElement(using: &config.RANDOM)!!)
                } else {
                    await waitForStyle()
                    waitedForInitialStyle = true
                }
                guard !stopped, !Task.isCancelled else { return }

                guard let routeFile = config.selectRandomRouteFile(),
                      let routeJson = config.getRouteJson(filename: routeFile)
                else { return }

                showTransientMessage("Loading '\(routeFile.lastPathComponent)' …")
                let route: NavigationRoute
                do {
                    route = try NavigationRoute(json: routeJson, mapView: self)
                } catch {
                    print("NavigationMap: \(error)")
                    showTransientMessage("Failed to load '\(routeFile.lastPathComponent)'", autoHideAfter: 3.0)
                    return
                }
                route.trimToVisibleExtent = trimToVisibleExtent
                self.route = route

                guard let locationManager = locationManager as? NavigationLocationManager else { return }

                let distStr = (route.distance / 1000).formatted(.number.precision(.fractionLength(2)))
                showTransientMessage("Loaded route from '\(routeFile.lastPathComponent)' with \(route.geometry.count) points, \(distStr) km", autoHideAfter: 3.0)

                let startingWaypoint = locationManager.getCoord(distance: 0.0)

                locationManager.navigationDelegate = self
                locationManager.speedMultiplier = userSpeedMultiplier ?? config.randomSpeed()
                let defaultInterval = trimRoute ? Self.trimRouteDefaultUpdateInterval : NavigationLocationManager.defaultUpdateInterval
                locationManager.updateInterval = userUpdateInterval ?? defaultInterval

                let camera = MLNMapCamera(
                    lookingAtCenter: startingWaypoint,
                    altitude: config.randomAltitude(),
                    pitch: config.randomTilt(),
                    heading: 0.0
                )

                setCamera(camera, animated: true)
                onRouteReady?()

                if delay {
                    try await Task.sleep(for: .seconds(config.randomWaitTime()))
                }
                guard !Task.isCancelled else { return }

                route.start()
            } catch {
                // cancelled while sleeping
            }
        }
    }

    func progressDidChange(currentDistance: Double, remainingDistance _: Double) {
        guard trimRoute else { return }
        route?.updateDisplayedGeometry(from: currentDistance)
    }

    func navigationDidComplete() {
        startNewRoute(nextStyle: true, delay: true)
    }
}

final class NavigationMapHandle {
    weak var map: NavigationMap?
}
