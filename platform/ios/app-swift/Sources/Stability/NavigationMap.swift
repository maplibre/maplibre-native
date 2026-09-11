import MapLibre

private class NavigationConfig {
    var RANDOM = RandomNumberGeneratorWithSeed(seed: 42)

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

    func getRouteResponseJson() -> [String: Any]? {
        var responseJson: [String: Any]?

        do {
            let routeDirectory = Bundle.main.resourcePath! + "/Routes/"
            let routeFile = try FileManager.default.contentsOfDirectory(atPath: routeDirectory).randomElement(using: &RANDOM)!
            let jsonStr = try String(contentsOfFile: routeDirectory + routeFile, encoding: .utf8)
            responseJson = try JSONSerialization.jsonObject(with: jsonStr.data(using: .utf8)!, options: []) as? [String: Any] ?? [:]
        } catch {
            print("NavigationMap: \(error)")
        }

        return responseJson
    }
}

class NavigationMap: MLNMapView, MLNMapViewDelegate, NavigationLocationManagerDelegate {
    fileprivate let config = NavigationConfig()
    private var route: NavigationRoute?
    private var styleGate: Gate?
    private var task: Task<Void, Never>?
    private var stopped = false

    init() {
        super.init(frame: CGRect())
    }

    @available(*, unavailable)
    @MainActor required init?(coder _: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    @MainActor func load(style: URL) async {
        let gate = Gate()
        styleGate = gate

        styleURL = style

        await gate.wait()
        styleGate = nil
    }

    @objc(mapView:didFinishLoadingStyle:) func mapView(_: MLNMapView, didFinishLoading _: MLNStyle) {
        styleGate?.open()
    }

    func mapViewDidFailLoadingMap(_: MLNMapView, withError error: Error) {
        // a failed style load never reports didFinishLoading
        print("NavigationMap: \(error)")
        styleGate?.open()
    }

    func run() {
        delegate = self

        startNewRoute()
    }

    func stop() {
        guard !stopped else { return }

        print("NavigationMap: stop")

        stopped = true
        task?.cancel()
        task = nil

        // release a pending style load so the task can observe the cancellation
        // and stop holding on to this map view
        styleGate?.open()

        // drops the simulated location manager, stopping its perform-based
        // update chain (which otherwise keeps the map view alive)
        route = nil
    }

    deinit {
        task?.cancel()
        print("NavigationMap: deinit")
    }

    func startNewRoute() {
        guard !stopped else { return }

        task?.cancel()
        task = Task { [weak self] in
            do {
                guard let waitTime = self?.config.randomWaitTime() else { return }
                try await Task.sleep(for: .seconds(waitTime))

                guard let self, !Task.isCancelled else { return }

                // reset existing route
                route = nil

                await load(style: config.STYLES.randomElement(using: &config.RANDOM)!!)
                guard !Task.isCancelled else { return }

                let routeJson = config.getRouteResponseJson()
                route = NavigationRoute(json: routeJson!, mapView: self)

                let locationManager = locationManager as! NavigationLocationManager
                let startingWaypoint = locationManager.getCoord(distance: 0.0)

                locationManager.navigationDelegate = self
                locationManager.speedMultiplier = config.randomSpeed()

                let camera = MLNMapCamera(
                    lookingAtCenter: startingWaypoint,
                    altitude: config.randomAltitude(),
                    pitch: config.randomTilt(),
                    heading: 0.0
                )

                setCamera(camera, animated: true)

                try await Task.sleep(for: .seconds(config.randomWaitTime()))
                guard !Task.isCancelled else { return }

                route?.start()
            } catch {
                // cancelled while sleeping
            }
        }
    }

    func progressDidChange(currentDistance _: Double, remainingDistance _: Double) {}

    func navigationDidComplete() {
        startNewRoute()
    }
}
