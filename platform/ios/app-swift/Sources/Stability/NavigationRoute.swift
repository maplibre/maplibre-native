import MapLibre
import Polyline
import UIKit

class NavigationRoute {
    let SourceIdentifier = "routeSource"
    let LayerIdentifier = "routeLayer"
    let BodySourceIdentifier = "routeBodySource"
    let BodyLayerIdentifier = "routeBodyLayer"
    let LineWidthByZoomLevel: [Int: Double] = [
        10: 12,
        13: 13.5,
        16: 16.5,
        19: 33,
        22: 42,
    ]

    weak var mapView: MLNMapView?
    private weak var installedLocationManager: NavigationLocationManager?
    private var destinationAnnotation: MLNPointAnnotation?
    var distance = 0.0
    var duration = 0.0
    var destination: CLLocationCoordinate2D?
    private(set) var path: RoutePath

    /// Coordinates currently in the route line
    var geometry: [CLLocationCoordinate2D] { path.coordinates }

    private(set) var displayedPointCount = 0

    /// When set, the line ahead of the puck is cut to the visible map extent.
    var trimToVisibleExtent = true
    private(set) var trimAnimationActive = false
    private var trimFromDistance = 0.0
    private var trimToDistance = 0.0
    private var trimStartedAt: CFTimeInterval = 0
    private var trimDuration: TimeInterval = 0
    /// Van Wijk matches camera `flyTo` when tracking; linear matches puck-only `UIView` animation.
    private var trimUseVanWijkEasing = false
    private var displayedTrimDistance: Double?
    private var appliedLookahead = -1.0
    /// First vertex of the cached full-route tail. `-1` forces the next rebuild.
    private var fullRouteJoinIndex = -1
    /// Progress in the current trim animation; never decreases within a step.
    private var lastTrimT: Double = 0
    /// Shape / paint updates can re-enter map camera/render callbacks on the same stack.
    private var isUpdatingDisplayedGeometry = false
    private var queuedTrimDistance: Double?

    // only set once on first use
    private static let polyline6: Void = {
        Polyline.setCompressionAlgorithm(CompressionAlgorithm.Polyline6)
    }()

    init(json: [String: Any], mapView: MLNMapView) throws {
        self.mapView = mapView
        path = RoutePath(coordinates: [])
        do {
            try load(json: json)
        } catch {
            unload()
            throw error
        }
    }

    deinit {
        stopTrimAnimation()
        // Only tear down location if we still own it. After `unload()`, the map
        // may already have a new route's manager; clearing that would freeze it.
        guard mapView?.locationManager === installedLocationManager else { return }
        mapView?.showsUserLocation = false
        installedLocationManager?.stopUpdatingLocation()
        mapView?.locationManager = nil
        if let destinationAnnotation {
            mapView?.removeAnnotation(destinationAnnotation)
        }
    }

    private func load(json: [String: Any]) throws {
        let routes = try Self.require(json["routes"] as? [[String: Any]], "Route JSON is missing routes")
        let routeJson = try Self.require(routes.first, "Route JSON has an empty routes array")

        distance = try Self.number(routeJson["distance"], "Route distance")
        duration = try Self.number(routeJson["duration"], "Route duration")

        let waypoints = try Self.require(json["waypoints"] as? [[String: Any]], "Route JSON is missing waypoints")
        let destinationJson = try Self.require(waypoints.last?["location"] as? [Double], "Route JSON is missing destination")
        guard destinationJson.count >= 2 else {
            throw Self.routeError("Route destination must be [lng, lat]")
        }
        destination = CLLocationCoordinate2D(latitude: destinationJson[1], longitude: destinationJson[0])

        path = try RoutePath(coordinates: Self.coordinates(from: routeJson))
        try loadGeometry(from: path.coordinates)

        assert(mapView?.locationManager as? NavigationLocationManager == nil, "A Navigation Location manager should not already be set")

        let locationManager = NavigationLocationManager(path: path)
        locationManager.speed = duration > 0 ? distance / duration : 20.0
        installedLocationManager = locationManager
        mapView?.locationManager = locationManager
    }

    private static func coordinates(from routeJson: [String: Any]) throws -> [CLLocationCoordinate2D] {
        _ = polyline6

        if let polyline = routeJson["geometry"] as? String {
            return try Polyline.decodeToLngLatArray(polyline).map {
                CLLocationCoordinate2D(latitude: $0.last!, longitude: $0.first!)
            }
        }

        if let geometry = routeJson["geometry"] as? [String: Any],
           let coordinates = geometry["coordinates"] as? [[Double]]
        {
            return try coordinates.map { coord in
                guard coord.count >= 2 else {
                    throw routeError("Route coordinate must be [lng, lat]")
                }
                return CLLocationCoordinate2D(latitude: coord[1], longitude: coord[0])
            }
        }

        throw routeError("Route geometry must be polyline6 or GeoJSON LineString")
    }

    private static func number(_ value: Any?, _ name: String) throws -> Double {
        if let number = value as? NSNumber {
            return number.doubleValue
        }
        throw routeError("\(name) must be a number")
    }

    private static func require<T>(_ value: T?, _ message: String) throws -> T {
        guard let value else { throw routeError(message) }
        return value
    }

    private static func routeError(_ message: String) -> NSError {
        NSError(domain: "NavigationRoute", code: 1, userInfo: [NSLocalizedDescriptionKey: message])
    }

    private func shape(from coordinates: [CLLocationCoordinate2D]) -> MLNShapeCollectionFeature {
        var coords = coordinates
        return MLNShapeCollectionFeature(shapes: [MLNPolylineFeature(coordinates: &coords, count: UInt(coords.count))])
    }

    func updateDisplayedGeometry(from currentDistance: Double) {
        guard !isUpdatingDisplayedGeometry else {
            queuedTrimDistance = currentDistance
            return
        }

        isUpdatingDisplayedGeometry = true
        defer {
            isUpdatingDisplayedGeometry = false
            if let queued = queuedTrimDistance {
                queuedTrimDistance = nil
                updateDisplayedGeometry(from: queued)
            }
        }

        let locationManager = mapView?.locationManager as? NavigationLocationManager
        let from = displayedTrimDistance ?? currentDistance
        trimFromDistance = from
        trimToDistance = max(currentDistance, from)
        let started = mapView?.userLocationAnimationReferenceTime ?? 0
        trimStartedAt = started > 0 ? started : CACurrentMediaTime()
        trimDuration = locationManager?.lastLocationAnimationDuration ?? 0
        trimUseVanWijkEasing = mapView?.userLocationCameraFollowsConstantZoomFly == true
        lastTrimT = 0
        trimAnimationActive = true

        applyTrim(distance: trimDistance(at: CACurrentMediaTime()))
    }

    /// Approximate scale factor from pixels to meters at the screen center
    private func metersPerPixel() -> Double {
        guard let mapView, mapView.bounds.width > 0 else { return 1 }
        let center = CGPoint(x: mapView.bounds.midX, y: mapView.bounds.midY)
        let offset = 10.0
        let a = mapView.convert(CGPoint(x: center.x - offset, y: center.y - offset), toCoordinateFrom: mapView)
        let b = mapView.convert(CGPoint(x: center.x + offset, y: center.y + offset), toCoordinateFrom: mapView)
        let meters = a.distance(to: b) / hypot(2 * offset, 2 * offset)
        return meters > 0 && meters.isFinite ? meters : 1
    }

    /// Estimate how far we need to look-ahead on the route for this view
    private func lookaheadMeters() -> Double {
        guard trimToVisibleExtent else { return .greatestFiniteMagnitude }
        guard let mapView else { return 500 }
        let bounds = mapView.visibleCoordinateBounds
        let diagonal = bounds.sw.distance(to: bounds.ne)
        let multiplier = 2.0
        return max(diagonal * multiplier, 250)
    }

    /// Determine the current trim distance
    private func trimDistance(at time: CFTimeInterval) -> Double {
        let interpolated: Double
        if trimDuration <= 0 || trimToDistance <= trimFromDistance {
            // not animating
            interpolated = trimToDistance
        } else {
            // animating
            var t = min(1, max(0, (time - trimStartedAt) / trimDuration))
            t = max(t, lastTrimT)
            lastTrimT = t
            let progress = trimUseVanWijkEasing ? Self.vanWijkGroundProgress(t) : t
            interpolated = trimFromDistance + (trimToDistance - trimFromDistance) * progress
        }
        // Ensure that the distance increases monotonically to avoid jitter/flicker
        return max(interpolated, displayedTrimDistance ?? interpolated)
    }

    /// The easing used by `flyTo` when zoom doesn't change
    private static func vanWijkGroundProgress(_ k: Double) -> Double {
        if k <= 0 { return 0 }
        if k >= 1 { return 1 }
        let r0 = log(sqrt(2.0) - 1.0)
        return 0.5 * (cosh(r0) * tanh(r0 + k * (-r0 - r0)) - sinh(r0))
    }

    /// Apply the trim to the route line
    private func applyTrim(distance: Double, force: Bool = false) {
        let distance = max(distance, displayedTrimDistance ?? 0)
        if trimToVisibleExtent {
            applyExtentTrim(distance: distance, force: force)
        } else {
            applyFullRouteTrim(distance: distance, force: force)
        }
    }

    /// Trim the route line to the visible extent
    private func applyExtentTrim(distance: Double, force: Bool) {
        guard let style = mapView?.style else { return }

        let lookahead = lookaheadMeters()
        let bodyPresent = style.source(withIdentifier: BodySourceIdentifier) != nil
        if !force,
           !bodyPresent,
           displayedTrimDistance == distance,
           style.source(withIdentifier: SourceIdentifier) != nil,
           abs(lookahead - appliedLookahead) <= max(appliedLookahead * 0.08, 25)
        {
            return
        }

        // not using split line, remove the body line layer
        removeBodyLine(from: style)
        appliedLookahead = lookahead
        displayedTrimDistance = distance
        let coordinates = displayedCoordinates(from: distance, lookahead: lookahead)
        displayedPointCount = coordinates.count
        ensureLine(coordinates, sourceIdentifier: SourceIdentifier, layerIdentifier: LayerIdentifier, on: style)
    }

    /// When showing the entire route line, the route is split so that only
    /// the first part needs to be updated on each frame.
    /// The tail is uploaded only when the join vertex changes.
    private func applyFullRouteTrim(distance: Double, force: Bool) {
        guard let style = mapView?.style else { return }
        appliedLookahead = -1

        let minLeading = min(metersPerPixel() * 2, 2.0)
        let lead = max(minLeading, 0.05)
        let endDistance = min(max(distance, 0) + lead, path.totalDistance)
        let suffixIndex = path.coordinates.isEmpty
            ? 0
            : min(path.vertexIndex(at: endDistance) + 1, path.coordinates.count - 1)
        let expectsBody = path.coordinates.count - suffixIndex >= 2
        let headMissing = style.source(withIdentifier: SourceIdentifier) == nil
        let bodyMissing = expectsBody && style.source(withIdentifier: BodySourceIdentifier) == nil
        let headChanged = force || headMissing || displayedTrimDistance != distance
        let bodyChanged = force || bodyMissing || suffixIndex != fullRouteJoinIndex
        guard headChanged || bodyChanged else { return }

        let head = displayedCoordinates(from: distance, lookahead: lead)
        displayedTrimDistance = distance
        if headChanged {
            ensureLine(head, sourceIdentifier: SourceIdentifier, layerIdentifier: LayerIdentifier, on: style)
        }
        if bodyChanged {
            fullRouteJoinIndex = suffixIndex
            let body = suffixIndex < path.coordinates.count ? Array(path.coordinates[suffixIndex...]) : []
            ensureLine(body, sourceIdentifier: BodySourceIdentifier, layerIdentifier: BodyLayerIdentifier, on: style)
        }
        displayedPointCount = head.count + max(0, path.coordinates.count - suffixIndex)
    }

    /// Rebuilds the line after the visible extent changes and the puck has not moved.
    /// An active step already applies the extent on each frame.
    func refreshVisibleExtentTrimIfNeeded() {
        guard trimToVisibleExtent, !trimAnimationActive, !isUpdatingDisplayedGeometry, displayedTrimDistance != nil else { return }
        isUpdatingDisplayedGeometry = true
        defer { isUpdatingDisplayedGeometry = false }
        applyTrim(distance: displayedTrimDistance ?? 0)
    }

    /// Applies `trimToVisibleExtent` to the line already on the map.
    func reapplyDisplayedGeometry() {
        guard !isUpdatingDisplayedGeometry else { return }
        isUpdatingDisplayedGeometry = true
        defer { isUpdatingDisplayedGeometry = false }
        applyTrim(distance: displayedTrimDistance ?? 0, force: true)
    }

    /// Puts the current line back after a style reload removes style sources.
    func reinstallDisplayedLine() {
        guard let style = mapView?.style else { return }
        removeLineLayers(from: style)
        fullRouteJoinIndex = -1
        appliedLookahead = -1
        if displayedTrimDistance != nil {
            applyTrim(distance: displayedTrimDistance ?? 0, force: true)
        } else {
            installLine(path.coordinates, on: style)
        }
    }

    private func displayedCoordinates(from distance: Double, lookahead: Double) -> [CLLocationCoordinate2D] {
        let minLeading = min(metersPerPixel() * 2, 2.0)
        return path.remainingCoordinates(from: distance, lookahead: lookahead, minLeading: minLeading)
    }

    func tickDisplayedGeometry(at time: CFTimeInterval = CACurrentMediaTime()) {
        guard trimAnimationActive, !isUpdatingDisplayedGeometry else { return }
        isUpdatingDisplayedGeometry = true
        defer { isUpdatingDisplayedGeometry = false }
        applyTrim(distance: trimDistance(at: time))
        if trimDuration <= 0 || time - trimStartedAt >= trimDuration {
            trimAnimationActive = false
        }
    }

    private func stopTrimAnimation() {
        trimAnimationActive = false
        displayedTrimDistance = nil
        appliedLookahead = -1
        queuedTrimDistance = nil
        trimFromDistance = 0
        trimToDistance = 0
        trimDuration = 0
        lastTrimT = 0
        trimUseVanWijkEasing = false
        fullRouteJoinIndex = -1
    }

    private func loadGeometry(from geometry: [CLLocationCoordinate2D]) throws {
        guard let style = mapView?.style else {
            throw Self.routeError("Map style is not loaded")
        }

        installLine(geometry, on: style)

        let annotation = MLNPointAnnotation()
        annotation.coordinate = try Self.require(destination, "Route destination is missing")
        mapView?.addAnnotation(annotation)
        destinationAnnotation = annotation
    }

    private func installLine(_ coordinates: [CLLocationCoordinate2D], on style: MLNStyle) {
        displayedPointCount = coordinates.count
        ensureLine(coordinates, sourceIdentifier: SourceIdentifier, layerIdentifier: LayerIdentifier, on: style)
    }

    private func ensureLine(
        _ coordinates: [CLLocationCoordinate2D],
        sourceIdentifier: String,
        layerIdentifier: String,
        on style: MLNStyle
    ) {
        guard coordinates.count >= 2 else {
            removeLineLayer(sourceIdentifier: sourceIdentifier, layerIdentifier: layerIdentifier, from: style)
            return
        }
        if let source = style.source(withIdentifier: sourceIdentifier) as? MLNShapeSource {
            source.shape = shape(from: coordinates)
            return
        }

        let source = MLNShapeSource(
            identifier: sourceIdentifier,
            shape: shape(from: coordinates),
            options: [MLNShapeSourceOption.synchronousUpdate: true]
        )
        style.addSource(source)

        let layer = MLNLineStyleLayer(identifier: layerIdentifier, source: source)
        layer.lineWidth = NSExpression(forMLNInterpolating: .zoomLevelVariable,
                                       curveType: .linear,
                                       parameters: nil,
                                       stops: NSExpression(forConstantValue: LineWidthByZoomLevel))
        layer.lineColor = NSExpression(forConstantValue: UIColor.blue)
        layer.lineOpacity = NSExpression(forConstantValue: 1)
        layer.lineCap = NSExpression(forConstantValue: "butt")
        layer.lineJoin = NSExpression(forConstantValue: "round")

        // Insert route layers above the last line layer so they're over roads, but (hopefully) below labels.
        if let lastLine = style.layers.last(where: { $0 is MLNLineStyleLayer }) {
            style.insertLayer(layer, above: lastLine)
        } else {
            style.addLayer(layer)
        }
    }

    private func removeBodyLine(from style: MLNStyle) {
        guard style.source(withIdentifier: BodySourceIdentifier) != nil ||
            style.layer(withIdentifier: BodyLayerIdentifier) != nil
        else { return }
        removeLineLayer(sourceIdentifier: BodySourceIdentifier, layerIdentifier: BodyLayerIdentifier, from: style)
        fullRouteJoinIndex = -1
    }

    private func removeLineLayers(from style: MLNStyle) {
        removeLineLayer(sourceIdentifier: BodySourceIdentifier, layerIdentifier: BodyLayerIdentifier, from: style)
        removeLineLayer(sourceIdentifier: SourceIdentifier, layerIdentifier: LayerIdentifier, from: style)
    }

    private func removeLineLayer(sourceIdentifier: String, layerIdentifier: String, from style: MLNStyle) {
        if let layer = style.layer(withIdentifier: layerIdentifier) {
            style.removeLayer(layer)
        }
        if let source = style.source(withIdentifier: sourceIdentifier) {
            style.removeSource(source)
        }
    }

    func start() {
        mapView?.userTrackingMode = .followWithCourse
        mapView?.showsUserLocation = true
    }

    func stop() {
        stopTrimAnimation()
        guard mapView?.locationManager === installedLocationManager else { return }
        installedLocationManager?.stopUpdatingLocation()
        mapView?.showsUserLocation = false
    }

    func unload() {
        stop()

        if mapView?.locationManager === installedLocationManager {
            mapView?.locationManager = nil
        }
        installedLocationManager = nil

        if let style = mapView?.style {
            removeLineLayers(from: style)
        }

        if let destinationAnnotation {
            mapView?.removeAnnotation(destinationAnnotation)
            self.destinationAnnotation = nil
        }
    }
}
