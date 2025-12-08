import MapLibre
import Polyline

@objc public protocol NavigationLocationManagerDelegate: AnyObject {
    @objc func progressDidChange(currentDistance: Double, remainingDistance: Double)
    @objc func navigationDidComplete()
}

class NavigationLocationManager: NSObject, MLNLocationManager {
    var delegate: (any MLNLocationManagerDelegate)?
    var authorizationStatus: CLAuthorizationStatus = .authorizedAlways
    var headingOrientation: CLDeviceOrientation = .portrait

    var navigationDelegate: (any NavigationLocationManagerDelegate)?
    var updateInterval = 0.2
    var speed = 20.0
    var speedMultiplier = 1.0

    var currentDistance = 0.0
    var totalDistance = 0.0
    var coordinates: [CLLocationCoordinate2D] = []

    init(coordinates: [CLLocationCoordinate2D]) {
        super.init()
        self.coordinates = coordinates

        totalDistance = zip(coordinates, coordinates.dropFirst())
            .map { $0.distance(to: $1) }
            .reduce(0.0, +)
    }

    func requestAlwaysAuthorization() {}
    func requestWhenInUseAuthorization() {}
    func dismissHeadingCalibrationDisplay() {}
    func startUpdatingHeading() {}
    func stopUpdatingHeading() {}

    func startUpdatingLocation() {
        DispatchQueue.main.async(execute: update)
    }

    func stopUpdatingLocation() {
        DispatchQueue.main.async {
            NSObject.cancelPreviousPerformRequests(withTarget: self, selector: #selector(self.update), object: nil)
        }
    }

    @objc private func update() {
        guard coordinates.count > 0 else { return }

        let currentCoord = getCoord(distance: currentDistance)
        let nextCoord = getCoord(distance: currentDistance + 10)

        let location = CLLocation(coordinate: currentCoord,
                                  altitude: 0.0,
                                  horizontalAccuracy: 40.0,
                                  verticalAccuracy: 10.0,
                                  course: currentCoord.direction(to: nextCoord).wrap(min: 0, max: 360),
                                  speed: speed,
                                  timestamp: Date())

        delegate?.locationManager(self, didUpdate: [location])
        navigationDelegate?.progressDidChange(currentDistance: currentDistance, remainingDistance: max(totalDistance - currentDistance, 0.0))

        if currentCoord.latitude == coordinates.last?.latitude,
           currentCoord.longitude == coordinates.last?.longitude
        {
            navigationDelegate?.navigationDidComplete()
            return
        }

        perform(#selector(update), with: nil, afterDelay: updateInterval)

        // can get a more dynamic speed value from the route steps
        currentDistance += speed * speedMultiplier * updateInterval
    }

    func getCoord(distance: Double) -> CLLocationCoordinate2D {
        var traveled = 0.0
        for i in 0 ..< coordinates.count {
            guard distance < traveled || i < coordinates.count - 1 else {
                break
            }

            if traveled >= distance {
                let overshoot = distance - traveled
                if overshoot == 0 {
                    return coordinates[i]
                }

                let direction = coordinates[i].direction(to: coordinates[i - 1]) - 180
                return coordinates[i].coordinate(at: overshoot, facing: direction)
            }

            traveled += coordinates[i].distance(to: coordinates[i + 1])
        }

        return coordinates.last!
    }
}

class NavigationRouteStep {
    var name: String?
    var duration = 0.0
    var distance = 0.0
    var maneuverType: String?
    var maneuverBearingBefore = 0
    var maneuverBearingAfter = 0
    var location = CLLocationCoordinate2D()

    init(json: [String: Any]) {
        name = json["name"] as? String
        duration = json["duration"] as! Double
        distance = json["distance"] as! Double

        let maneuverJson = json["maneuver"] as! [String: Any]
        maneuverType = maneuverJson["type"] as? String
        maneuverBearingBefore = maneuverJson["bearing_before"] as! Int
        maneuverBearingAfter = maneuverJson["bearing_after"] as! Int

        let locationJson = maneuverJson["location"] as! [Double]
        location = CLLocationCoordinate2D(latitude: locationJson.last!, longitude: locationJson.first!)
    }
}

class NavigationRoute {
    let SourceIdentifier = "routeSource"
    let LayerIdentifier = "routeLayer"
    let LineWidthByZoomLevel: [Int: Double] = [
        10: 12,
        13: 13.5,
        16: 16.5,
        19: 33,
        22: 42,
    ]

    var mapView: MLNMapView?
    var distance = 0.0
    var duration = 0.0
    var destination: CLLocationCoordinate2D?

    var steps: [NavigationRouteStep]?

    init(json: [String: Any], mapView: MLNMapView) {
        self.mapView = mapView

        load(json: json)
    }

    deinit {
        mapView?.showsUserLocation = false
        mapView?.locationManager = nil
    }

    private func load(json: [String: Any]) {
        let routeJson = (json["routes"] as! [[String: Any]]).first!

        // stats
        distance = routeJson["distance"] as! Double
        duration = routeJson["duration"] as! Double

        let waypointsJson = json["waypoints"] as! [[String: Any]]
        let destinationJson = waypointsJson.last!["location"] as! [Double]
        destination = CLLocationCoordinate2D(latitude: destinationJson.last!, longitude: destinationJson.first!)

        // route geometry
        let geometry: [LocationCoordinate2D] = decodePolyline(routeJson["geometry"] as! String, precision: 1e6)!

        loadGeometry(from: geometry)

        let locationManager = NavigationLocationManager(coordinates: geometry)
        locationManager.speed = distance / duration
        mapView?.locationManager = locationManager

        // route steps
        let legsJson = routeJson["legs"] as! [[String: Any]]
        let stepsJson = legsJson.first!["steps"] as! [[String: Any]]
        steps = stepsJson.map { NavigationRouteStep(json: $0) }
    }

    private func loadGeometry(from geometry: [CLLocationCoordinate2D]) {
        let polylines = MLNShapeCollectionFeature(shapes: [MLNPolylineFeature(coordinates: geometry, count: UInt(geometry.count))])
        let source = MLNShapeSource(identifier: SourceIdentifier, shape: polylines, options: nil)
        mapView?.style?.addSource(source)

        let layer = MLNLineStyleLayer(identifier: LayerIdentifier, source: source)
        layer.lineWidth = NSExpression(forMLNInterpolating: .zoomLevelVariable,
                                       curveType: .linear,
                                       parameters: nil,
                                       stops: NSExpression(forConstantValue: LineWidthByZoomLevel))

        layer.lineColor = NSExpression(forConstantValue: UIColor.blue)
        layer.lineOpacity = NSExpression(forConstantValue: 1)

        layer.lineCap = NSExpression(forConstantValue: "round")
        layer.lineJoin = NSExpression(forConstantValue: "round")

        for value in mapView!.style!.layers.reversed() {
            if !(value is MLNSymbolStyleLayer) {
                mapView?.style?.insertLayer(layer, below: value)
                break
            }
        }

        // destination waypoint annotation
        let annotation = MLNPointAnnotation()
        annotation.coordinate = destination!
        mapView?.addAnnotation(annotation)
    }

    func start() {
        mapView?.userTrackingMode = .followWithCourse
        mapView?.showsUserLocation = true
    }
}
