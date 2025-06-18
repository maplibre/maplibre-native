import MapboxCoreNavigation
import MapboxDirections
import MapboxNavigation
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

    let ROUTE_UPDATE_INTERVAL = 10.0

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
        random(in: 0.8 ... 3.0)
    }

    func getRoute() -> Route? {
        var responseJson: [String: Any]?

        do {
            let routeDirectory = Bundle.main.resourcePath! + "/Routes/"
            let routeFile = try FileManager.default.contentsOfDirectory(atPath: routeDirectory).randomElement(using: &RANDOM)!
            let jsonStr = try String(contentsOfFile: routeDirectory + routeFile, encoding: .utf8)
            responseJson = try JSONSerialization.jsonObject(with: jsonStr.data(using: .utf8)!, options: []) as? [String: Any] ?? [:]
        } catch {
            print("NavigationMap: \(error)")
        }

        if responseJson == nil {
            return nil
        }

        // get route from json response
        let routeJson = (responseJson!["routes"] as! [[String: Any]]).first!

        // get waypoints from json response
        let waypointsJson = responseJson!["waypoints"] as! [[String: Any]]
        let startLocationJson = waypointsJson.first!["location"] as! [Double]
        let endLocationJson = waypointsJson.last!["location"] as! [Double]

        let waypoints: [Waypoint] = [
            Waypoint(coordinate: CLLocationCoordinate2D(
                latitude: startLocationJson.last!,
                longitude: startLocationJson.first!
            )),
            Waypoint(coordinate: CLLocationCoordinate2D(
                latitude: endLocationJson.last!,
                longitude: endLocationJson.first!
            )),
        ]

        let routeOptions = NavigationRouteOptions(waypoints: waypoints, profileIdentifier: .automobile)
        routeOptions.shapeFormat = .polyline6
        routeOptions.distanceMeasurementSystem = .metric
        routeOptions.attributeOptions = []

        return Route(json: routeJson, waypoints: waypoints, options: routeOptions)
    }
}

class StandardNavigationMap: NavigationViewController {
    fileprivate let config = NavigationConfig()

    init() {
        super.init(dayStyle: DayStyle(mapStyleURL: config.STYLES.randomElement(using: &config.RANDOM)!!), nightStyle: NightStyle(demoStyle: ()))
    }

    @MainActor @objc(initWithDayStyle:nightStyle:directions:voiceController:) required init(dayStyle: Style, nightStyle: Style? = nil, directions: Directions = Directions.shared, voiceController: RouteVoiceController = RouteVoiceController()) {
        super.init(dayStyle: dayStyle, nightStyle: nightStyle, directions: directions, voiceController: voiceController)
    }

    func run() {
        automaticallyAdjustsStyleForTimeOfDay = false
        showsEndOfRouteFeedback = false

        mapView.tracksUserCourse = false
        mapView.showsUserLocation = true

        startNewRoute()
    }

    func startNewRoute() {
        Task { @MainActor in
            try await Task.sleep(for: .seconds(config.randomWaitTime()))

            mapView.styleURL = config.STYLES.randomElement(using: &config.RANDOM)!!

            try await Task.sleep(for: .seconds(config.randomWaitTime()))

            let route = config.getRoute()

            let simulatedLocationManager = SimulatedLocationManager(route: route!)
            simulatedLocationManager.speedMultiplier = config.randomSpeed()

            startNavigation(with: route!, animated: true, locationManager: simulatedLocationManager)
        }
    }

    func navigationViewControllerDidFinishRouting(_ navigationViewController: NavigationViewController) {
        navigationViewController.endNavigation()

        startNewRoute()
    }
}

class SimpleNavigationMap: NavigationMapView, MLNMapViewDelegate, RouteControllerDelegate {
    fileprivate let config = NavigationConfig()
    private var continuation: CheckedContinuation<Void, Never>?
    private var routeController: RouteController?

    init() {
        super.init(frame: CGRect())

        NotificationCenter.default.addObserver(self, selector: #selector(progressDidChange(notification:)), name: .routeControllerProgressDidChange, object: routeController)
    }

    deinit {
        NotificationCenter.default.removeObserver(self, name: .routeControllerProgressDidChange, object: self.routeController)
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

            await MainActor.run {
                removeRoutes()
                removeWaypoints()
            }

            await load(style: config.STYLES.randomElement(using: &config.RANDOM)!!)

            let route = config.getRoute()

            showRoutes([route!])
            showWaypoints(route!)
            // showVoiceInstructionsOnMap(route: route!)

            let simulatedLocationManager = SimulatedLocationManager(route: route!)
            simulatedLocationManager.speedMultiplier = config.randomSpeed()

            routeController = RouteController(along: route!, locationManager: simulatedLocationManager)

            routeController?.delegate = self
            tracksUserCourse = true

            let startingWaypoint = route!.routeOptions.waypoints.first!.coordinate
            let startingLocation = CLLocation(latitude: startingWaypoint.latitude, longitude: startingWaypoint.longitude)

            defaultAltitude = config.randomAltitude()

            updateCourseTracking(location: startingLocation, animated: false)
            setCamera(MLNMapCamera(lookingAtCenter: startingWaypoint, altitude: defaultAltitude, pitch: 45.0, heading: 0.0), animated: true)

            try await Task.sleep(for: .seconds(config.randomWaitTime()))

            routeController?.resume()
        }
    }

    @objc func progressDidChange(notification: NSNotification) {
        let routeProgress = notification.userInfo![RouteControllerNotificationUserInfoKey.routeProgressKey] as! RouteProgress
        let location = notification.userInfo![RouteControllerNotificationUserInfoKey.locationKey] as! CLLocation

        guard let routeController else { return }

        // If the user has arrived, don't snap the user puck.
        // In the case the user drives beyond the waypoint,
        // we should accurately depict this.
        let shouldPreventReroutesWhenArrivingAtWaypoint = routeController.delegate?.routeController?(routeController, shouldPreventReroutesWhenArrivingAt: routeController.routeProgress.currentLeg.destination) ?? true
        let userHasArrivedAndShouldPreventRerouting = shouldPreventReroutesWhenArrivingAtWaypoint && !routeController.routeProgress.currentLegProgress.userHasArrivedAtWaypoint

        if userHasArrivedAndShouldPreventRerouting {
            updateCourseTracking(location: location, animated: true)
        }
    }

    func routeController(_ routeController: RouteController, didArriveAt _: Waypoint) -> Bool {
        if routeController.routeProgress.remainingWaypoints.count == 1 {
            startNewRoute()
        }

        return true
    }
}
