import MapLibre

private class NavigationConfig {
    var RANDOM = RandomNumberGeneratorWithSeed(seed: 42)

    let STYLES = [
        AMERICANA_STYLE,
        OPENFREEMAP_LIBERTY_STYLE,
        OPENFREEMAP_BRIGHT_STYLE,
        AWS_OPEN_DATA_STANDARD_LIGHT_STYLE,
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
    private var continuation: CheckedContinuation<Void, Never>?

    init() {
        super.init(frame: CGRect())
    }

    @available(*, unavailable)
    @MainActor required init?(coder _: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    @MainActor func load(style: URL) async {
        styleURL = style

        return await withCheckedContinuation { continuation in
            guard self.style == nil else {
                self.continuation = nil
                continuation.resume()
                return
            }

            self.continuation = continuation
        }
    }

    @objc(mapView:didFinishLoadingStyle:) func mapView(_: MLNMapView, didFinishLoading _: MLNStyle) {
        continuation?.resume()
        continuation = nil
    }

    func run() {
        delegate = self

        startNewRoute()
    }

    func startNewRoute() {
        Task {
            try await Task.sleep(for: .seconds(config.randomWaitTime()))

            // reset existing route
            route = nil

            await load(style: config.STYLES.randomElement(using: &config.RANDOM)!!)

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

            route!.start()
        }
    }

    func progressDidChange(currentDistance _: Double, remainingDistance _: Double) {}

    func navigationDidComplete() {
        startNewRoute()
    }
}
