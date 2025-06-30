import MapLibre

class UserMap: MLNMapView, MLNMapViewDelegate {
    private var continuation: CheckedContinuation<Void, Never>?

    // config
    private var RANDOM = RandomNumberGeneratorWithSeed(seed: 42)

    private let STYLES = [
        DEMOTILES_STYLE,
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

    private let PLACES = [
        CLLocationCoordinate2D(latitude: 37.7749, longitude: -122.4194), // SF
        CLLocationCoordinate2D(latitude: 38.9072, longitude: -77.0369), // DC
        CLLocationCoordinate2D(latitude: 52.3702, longitude: 4.8952), // AMS
        CLLocationCoordinate2D(latitude: 60.1699, longitude: 24.9384), // HEL
        CLLocationCoordinate2D(latitude: -13.1639, longitude: -74.2236), // AYA
        CLLocationCoordinate2D(latitude: 52.5200, longitude: 13.4050), // BER
        CLLocationCoordinate2D(latitude: 12.9716, longitude: 77.5946), // BAN
        CLLocationCoordinate2D(latitude: 31.2304, longitude: 121.4737), // SHA
    ]

    func random(in range: ClosedRange<Int>) -> Int {
        Int.random(in: range, using: &RANDOM)
    }

    func random(in range: ClosedRange<Double>) -> Double {
        Double.random(in: range, using: &RANDOM)
    }

    func weightedRandom<T>(in values: [(T, Double)]) -> T {
        let cumulativeWeights = values.reduce(into: (total: 0.0, result: [Double]())) { acc, value in
            acc.total += value.1
            acc.result.append(acc.total)
        }.result

        let randomWeight = random(in: 0.0 ... cumulativeWeights.last!)

        var low = 0
        var high = cumulativeWeights.count - 1

        while low < high {
            let mid = (low + high) / 2
            if randomWeight < cumulativeWeights[mid] {
                high = mid
            } else {
                low = mid + 1
            }
        }

        return values[low].0
    }

    func randomSlowDuration() -> TimeInterval { random(in: 3000 ... 5000) / 1000.0 }
    func randomFastDuration() -> TimeInterval { random(in: 500 ... 1000) / 1000.0 }

    func randomPlacePoints() -> Int { random(in: 10 ... 20) }
    func randomPlaceActions() -> Int { random(in: 10 ... 20) }

    func randomLatLng() -> CLLocationCoordinate2D {
        let bounds = visibleCoordinateBounds
        return CLLocationCoordinate2D(
            latitude: random(in: bounds.sw.latitude ... bounds.ne.latitude),
            longitude: random(in: bounds.sw.longitude ... bounds.ne.longitude)
        )
    }

    func randomZoom() -> Double { random(in: 14.0 ... 20.0) }

    func randomAltitude() -> Double {
        MLNAltitudeForZoomLevel(randomZoom(), camera.pitch, camera.centerCoordinate.latitude, frame.size)
    }

    func randomTilt() -> Double { random(in: 0.0 ... 60.0) }
    func randomBearing() -> Double { random(in: 0.0 ... 360.0) }

    func randomAnnotationRemove() -> Int { random(in: 4 ... 8) }
    func randomAnnotationAdd() -> Int { random(in: 2 ... 4) }

    func randomPolyPoints() -> [CLLocationCoordinate2D] {
        let bounds = visibleCoordinateBounds

        let boundMultiplier = 0.25
        let lat = random(in: bounds.sw.latitude ... bounds.ne.latitude)
        let lng = random(in: bounds.sw.longitude ... bounds.ne.longitude)
        let latRange = abs(bounds.ne.latitude - bounds.sw.latitude) * boundMultiplier / 2.0
        let lngRange = abs(bounds.ne.longitude - bounds.sw.longitude) * boundMultiplier / 2.0

        return (0 ... random(in: 3 ... 15)).map { _ in
            CLLocationCoordinate2D(
                latitude: lat + random(in: -latRange ... latRange),
                longitude: lng + random(in: -lngRange ... lngRange)
            )
        }
    }

    func randomLineWidth() -> Double { random(in: 1.0 ... 4.0) }

    func randomColor() -> UIColor {
        UIColor(
            red: CGFloat(random(in: 0.0 ... 1.0)),
            green: CGFloat(random(in: 0.0 ... 1.0)),
            blue: CGFloat(random(in: 0.0 ... 1.0)),
            alpha: CGFloat(random(in: 0.5 ... 1.0))
        )
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

    func mapView(_: MLNMapView, didFinishLoading _: MLNStyle) {
        continuation?.resume()
        continuation = nil
    }

    func mapView(_ mapView: MLNMapView, viewFor annotation: MLNAnnotation) -> MLNAnnotationView? {
        guard annotation is MLNPointAnnotation else {
            return nil
        }

        let reuseIdentifier = "(\(annotation.coordinate.latitude),\(annotation.coordinate.longitude))"
        let annotationView = mapView.dequeueReusableAnnotationView(withIdentifier: reuseIdentifier)
        if annotationView != nil {
            return annotationView
        }

        class AnnotationView: MLNAnnotationView {
            var map: UserMap

            init(map: UserMap, reuseIdentifier: String) {
                self.map = map
                super.init(reuseIdentifier: reuseIdentifier)
            }

            @available(*, unavailable)
            required init?(coder _: NSCoder) {
                fatalError("init(coder:) has not been implemented")
            }

            override func layoutSubviews() {
                super.layoutSubviews()

                layer.cornerRadius = bounds.width / 2
                layer.borderWidth = 2
                layer.borderColor = UIColor.white.cgColor

                layer.backgroundColor = map.randomColor().cgColor
            }
        }

        let annotation = AnnotationView(map: self, reuseIdentifier: reuseIdentifier)
        annotation.bounds = CGRect(x: 0, y: 0, width: 20, height: 20)
        return annotation
    }

    func mapView(_: MLNMapView, strokeColorForShapeAnnotation _: MLNShape) -> UIColor {
        randomColor()
    }

    func mapView(_: MLNMapView, lineWidthForPolylineAnnotation _: MLNPolyline) -> CGFloat {
        randomLineWidth()
    }

    func mapView(_: MLNMapView, fillColorForPolygonAnnotation _: MLNPolygon) -> UIColor {
        randomColor()
    }

    func run() {
        delegate = self

        Task {
            while true {
                await runStyleActions()
            }
        }
    }

    @MainActor func runStyleActions() async {
        for style in STYLES.shuffled(using: &RANDOM) {
            await load(style: style!)
            await runCameraActions()
        }
    }

    @MainActor func runCameraActions() async {
        // camera actions with different weights
        // position updates are more frequent
        let actions = [
            ({ [weak self] () in let cam = self!.camera; cam.centerCoordinate = self!.randomLatLng(); return cam }, 2.0),
            ({ [weak self] () in let cam = self!.camera; cam.altitude = self!.randomAltitude(); return cam }, 1.0),
            ({ [weak self] () in let cam = self!.camera; cam.pitch = self!.randomTilt(); return cam }, 1.0),
            ({ [weak self] () in let cam = self!.camera; cam.heading = self!.randomBearing(); return cam }, 1.0),
        ]

        for placeCenter in PLACES.shuffled(using: &RANDOM) {
            // update all values to simulate a long jump
            // (generated by the app, searching for a city/street, etc)
            let camera = MLNMapCamera(
                lookingAtCenter: placeCenter,
                altitude: randomAltitude(),
                pitch: randomTilt(),
                heading: randomBearing()
            )

            await animate(camera: camera, withDuration: randomSlowDuration())

            await randomPlacePoints().repeat {
                // perform a series of fast camera actions
                await randomPlaceActions().repeat {
                    // update each value individually to simulate user interaction
                    await self.animate(camera: weightedRandom(in: actions)(), withDuration: randomFastDuration())
                }

                runAnnotationActions()
            }
        }

        removeAnnotations(annotations ?? [])
    }

    func runAnnotationActions() {
        // remove some annotations
        min(annotations?.count ?? 0, randomAnnotationRemove()).repeat {
            self.removeAnnotation((self.annotations!.randomElement(using: &RANDOM)!))
        }

        // add some annotations

        // points
        randomAnnotationAdd().repeat {
            let annotation = MLNPointAnnotation()
            annotation.coordinate = randomLatLng()
            annotation.title = "(\(annotation.coordinate.latitude),\(annotation.coordinate.longitude))"

            self.addAnnotation(annotation)
        }

        // polylines
        randomAnnotationAdd().repeat {
            let coords = randomPolyPoints()
            let annotation = MLNPolyline(coordinates: coords, count: UInt(coords.count))

            // random color/width set via delegate
            self.addAnnotation(annotation)
        }

        // polygons
        randomAnnotationAdd().repeat {
            let coords = randomPolyPoints()
            let annotation = MLNPolygon(coordinates: coords, count: UInt(coords.count))

            // random fill/stroke colors set via delegate
            self.addAnnotation(annotation)
        }
    }
}
