import MapLibre
import UIKit

@objc public protocol NavigationLocationManagerDelegate: AnyObject {
    @objc func progressDidChange(currentDistance: Double, remainingDistance: Double)
    @objc func navigationDidComplete()
}

class NavigationLocationManager: NSObject, MLNLocationManager {
    weak var delegate: (any MLNLocationManagerDelegate)?
    weak var navigationDelegate: (any NavigationLocationManagerDelegate)?

    var authorizationStatus: CLAuthorizationStatus = .authorizedAlways
    var headingOrientation: CLDeviceOrientation = .portrait
    var updateInterval = defaultUpdateInterval
    var speed = 20.0 // m/s
    var speedMultiplier = 1.0
    var currentDistance = 0.0
    let path: RoutePath
    var totalDistance: Double { path.totalDistance }
    var coordinates: [CLLocationCoordinate2D] { path.coordinates }
    private(set) var lastLocationAnimationDuration: TimeInterval = 0
    private var lastLocationTimestamp: Date?

    static let defaultUpdateInterval = 0.2
    static let userLocationAnimationDuration: TimeInterval = 1.0

    init(path: RoutePath) {
        self.path = path
        super.init()
    }

    func requestAlwaysAuthorization() {}
    func requestWhenInUseAuthorization() {}
    func dismissHeadingCalibrationDisplay() {}
    func startUpdatingHeading() {}
    func stopUpdatingHeading() {}

    func startUpdatingLocation() {
        if Thread.isMainThread {
            update()
        } else {
            DispatchQueue.main.async(execute: update)
        }
    }

    func stopUpdatingLocation() {
        let cancel = {
            NSObject.cancelPreviousPerformRequests(withTarget: self, selector: #selector(self.update), object: nil)
        }
        if Thread.isMainThread {
            cancel()
        } else {
            DispatchQueue.main.async(execute: cancel)
        }
    }

    @objc private func update() {
        guard !coordinates.isEmpty else { return }

        let currentCoord = getCoord(distance: currentDistance)
        let nextCoord = getCoord(distance: currentDistance + 10)
        let timestamp = Date()
        let interval: TimeInterval
        if let lastLocationTimestamp {
            interval = timestamp.timeIntervalSince(lastLocationTimestamp)
        } else {
            interval = updateInterval
        }

        lastLocationAnimationDuration = min(interval, Self.userLocationAnimationDuration)
        lastLocationTimestamp = timestamp

        let location = CLLocation(coordinate: currentCoord,
                                  altitude: 0.0,
                                  horizontalAccuracy: 40.0,
                                  verticalAccuracy: 10.0,
                                  course: currentCoord.direction(to: nextCoord).wrap(min: 0, max: 360),
                                  speed: speed,
                                  timestamp: timestamp)

        delegate?.locationManager(self, didUpdate: [location])
        navigationDelegate?.progressDidChange(
            currentDistance: currentDistance,
            remainingDistance: max(totalDistance - currentDistance, 0.0)
        )

        if currentDistance >= totalDistance {
            navigationDelegate?.navigationDidComplete()
            return
        }

        perform(#selector(update), with: nil, afterDelay: updateInterval)
        currentDistance += speed * speedMultiplier * updateInterval
    }

    func getCoord(distance: Double) -> CLLocationCoordinate2D {
        path.point(at: distance)
    }
}
