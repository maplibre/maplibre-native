import MapLibre
import ObjectiveC
import UIKit

private enum NavigationMapGestureAssociatedKeys {
    static var state: UInt8 = 0
}

private final class NavigationMapGestureState {
    enum RouteControlPanAxis {
        case undecided
        case vertical
        case horizontal
    }

    var userSpeedMultiplier: Double?
    var speedMultiplierAtGestureStart: Double = 1.0
    var userUpdateInterval: TimeInterval?
    var updateIntervalAtGestureStart: TimeInterval = NavigationLocationManager.defaultUpdateInterval
    var routeControlPanAxis: RouteControlPanAxis = .undecided
}

extension NavigationMap {
    private static let minSpeedMultiplier = 0.1
    private static let maxSpeedMultiplier = 10.0
    private static let minUpdateRateHz = 0.5
    private static let maxUpdateRateHz = 60.0
    private static let controlSwipePointsPerDecade: CGFloat = 280
    private static let routeControlPanAxisLockThreshold: CGFloat = 12

    private var gestureState: NavigationMapGestureState {
        if let state = objc_getAssociatedObject(self, &NavigationMapGestureAssociatedKeys.state) as? NavigationMapGestureState {
            return state
        }
        let state = NavigationMapGestureState()
        objc_setAssociatedObject(
            self,
            &NavigationMapGestureAssociatedKeys.state,
            state,
            .OBJC_ASSOCIATION_RETAIN_NONATOMIC
        )
        return state
    }

    private(set) var userSpeedMultiplier: Double? {
        get { gestureState.userSpeedMultiplier }
        set { gestureState.userSpeedMultiplier = newValue }
    }

    private(set) var userUpdateInterval: TimeInterval? {
        get { gestureState.userUpdateInterval }
        set { gestureState.userUpdateInterval = newValue }
    }

    private var speedMultiplierAtGestureStart: Double {
        get { gestureState.speedMultiplierAtGestureStart }
        set { gestureState.speedMultiplierAtGestureStart = newValue }
    }

    private var updateIntervalAtGestureStart: TimeInterval {
        get { gestureState.updateIntervalAtGestureStart }
        set { gestureState.updateIntervalAtGestureStart = newValue }
    }

    private var routeControlPanAxis: NavigationMapGestureState.RouteControlPanAxis {
        get { gestureState.routeControlPanAxis }
        set { gestureState.routeControlPanAxis = newValue }
    }

    /// If the user interacts with the map, `userTrackingMode` will be disabled.
    /// Add a tap gesture recognizer to re-enable it.
    func addRestoreTrackingTapGesture() {
        let tap = UITapGestureRecognizer(target: self, action: #selector(restoreLocationTracking))
        if let recognizers = gestureRecognizers {
            for recognizer in recognizers {
                guard let tapRecognizer = recognizer as? UITapGestureRecognizer,
                      tapRecognizer.numberOfTapsRequired == 2
                else { continue }
                tap.require(toFail: tapRecognizer)
            }
        }
        addGestureRecognizer(tap)
    }

    @objc func restoreLocationTracking() {
        guard showsUserLocation else { return }
        userTrackingMode = .followWithCourse
    }

    /// Three-finger tap skips to the next route. Vertical three-finger swipe
    /// adjusts speed; horizontal swipe adjusts the location update rate.
    func addRouteControlGestures() {
        let skipTap = UITapGestureRecognizer(target: self, action: #selector(skipToNextRoute))
        skipTap.numberOfTouchesRequired = 3

        let controlPan = UIPanGestureRecognizer(target: self, action: #selector(handleRouteControlPan(_:)))
        controlPan.minimumNumberOfTouches = 3
        controlPan.maximumNumberOfTouches = 3

        skipTap.require(toFail: controlPan)
        addGestureRecognizer(skipTap)
        addGestureRecognizer(controlPan)
    }

    @objc func skipToNextRoute() {
        showTransientMessage("Skipping to next route …")
        startNewRoute(nextStyle: false, delay: false)
    }

    @objc private func handleRouteControlPan(_ pan: UIPanGestureRecognizer) {
        switch pan.state {
        case .began:
            routeControlPanAxis = .undecided
            speedMultiplierAtGestureStart = currentSpeedMultiplier()
            updateIntervalAtGestureStart = currentUpdateInterval()
        case .changed, .ended, .cancelled:
            let translation = pan.translation(in: self)
            if routeControlPanAxis == .undecided {
                let absX = abs(translation.x)
                let absY = abs(translation.y)
                guard max(absX, absY) >= Self.routeControlPanAxisLockThreshold else { return }
                routeControlPanAxis = absX > absY ? .horizontal : .vertical
            }

            let hideAfter: TimeInterval? = (pan.state == .changed) ? nil : 2.0
            switch routeControlPanAxis {
            case .vertical:
                let decades = Double(-translation.y / Self.controlSwipePointsPerDecade)
                let value = min(
                    max(speedMultiplierAtGestureStart * pow(10, decades), Self.minSpeedMultiplier),
                    Self.maxSpeedMultiplier
                )
                applySpeedMultiplier(value)
                showTransientMessage("Speed \(formatSpeedMultiplier(value))", autoHideAfter: hideAfter)
            case .horizontal:
                let startHz = 1.0 / updateIntervalAtGestureStart
                let decades = Double(translation.x / Self.controlSwipePointsPerDecade)
                let hz = min(
                    max(startHz * pow(10, decades), Self.minUpdateRateHz),
                    Self.maxUpdateRateHz
                )
                applyUpdateInterval(1.0 / hz)
                showTransientMessage("Update \(formatUpdateRate(hz))", autoHideAfter: hideAfter)
            case .undecided:
                break
            }
        default:
            break
        }
    }

    private func currentSpeedMultiplier() -> Double {
        if let locationManager = locationManager as? NavigationLocationManager {
            return locationManager.speedMultiplier
        }
        return userSpeedMultiplier ?? 1.0
    }

    private func applySpeedMultiplier(_ value: Double) {
        userSpeedMultiplier = value
        (locationManager as? NavigationLocationManager)?.speedMultiplier = value
    }

    private func formatSpeedMultiplier(_ value: Double) -> String {
        if value >= 9.95 {
            return String(format: "%.0f×", value)
        }
        return String(format: "%.1f×", value)
    }

    private func currentUpdateInterval() -> TimeInterval {
        if let locationManager = locationManager as? NavigationLocationManager {
            return locationManager.updateInterval
        }
        return userUpdateInterval ?? NavigationLocationManager.defaultUpdateInterval
    }

    private func applyUpdateInterval(_ value: TimeInterval) {
        userUpdateInterval = value
        (locationManager as? NavigationLocationManager)?.updateInterval = value
    }

    private func formatUpdateRate(_ hz: Double) -> String {
        if hz >= 9.95 {
            return String(format: "%.0f Hz", hz)
        }
        if hz >= 0.95 {
            return String(format: "%.1f Hz", hz)
        }
        return String(format: "%.2f Hz", hz)
    }
}
