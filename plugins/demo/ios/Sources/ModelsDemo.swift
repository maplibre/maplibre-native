import MapLibre
import SwiftUI

/// The `gltf` layer: `.glb` models drawn at the points of a source. A feature picks its model with `model-uri`, and
/// heading, scale, altitude and opacity are paint properties evaluated every frame.
///
/// The scene is a ring road with three cars on it, trees, and a few extruded buildings. The cars and trees are depth
/// tested against the buildings (`model-occlusion`), so a car passing behind one is hidden.
///
/// The trees' `model-scale` reads their `scale` property, so the slider only edits the trees' data.
struct ModelsDemoView: View {
    @StateObject private var status = DemoStatus()
    @State private var treeSize = City.defaultTreeSize

    var body: some View {
        ZStack(alignment: .top) {
            ModelsMap(treeSize: treeSize, status: status)
            StatusBanner(status: status)
        }
        .safeAreaInset(edge: .bottom, spacing: 0) {
            ControlBar {
                HStack {
                    Text("Tree size").frame(width: 70, alignment: .leading)
                    Slider(value: $treeSize, in: City.treeSizes)
                    Text("\(treeSize, specifier: "%.1f")×").monospacedDigit().frame(width: 40)
                }
                .font(.subheadline)
            }
        }
    }
}

private struct ModelsMap: UIViewRepresentable {
    var treeSize: Double
    var status: DemoStatus

    func makeCoordinator() -> Coordinator { Coordinator(status: status) }

    func makeUIView(context: Context) -> MLNMapView {
        let mapView = MLNMapView.demoMap(style: "models")
        mapView.delegate = context.coordinator
        return mapView
    }

    func updateUIView(_: MLNMapView, context: Context) {
        context.coordinator.apply(treeSize: treeSize)
    }

    static func dismantleUIView(_: MLNMapView, coordinator: Coordinator) {
        coordinator.stop()
    }

    @MainActor
    final class Coordinator: NSObject, @preconcurrency MLNMapViewDelegate {
        let status: DemoStatus
        private var style: MLNStyle?
        private var timer: Timer?
        private var start = CACurrentMediaTime()
        private var treeSize = City.defaultTreeSize

        init(status: DemoStatus) { self.status = status }

        func mapView(_ mapView: MLNMapView, didFinishLoading style: MLNStyle) {
            self.style = style
            mapView.look(at: City.frame.origin, zoom: 16.9, pitch: 58, heading: 12)
            City.fillSources(of: style, treeSize: treeSize, status: status)
            start = CACurrentMediaTime()
            timer?.invalidate()
            timer = Timer.scheduledTimer(withTimeInterval: 1.0 / 20, repeats: true) { [weak self] _ in
                MainActor.assumeIsolated { self?.tick() }
            }
        }

        func mapViewDidFailLoadingMap(_: MLNMapView, withError error: Error) {
            status.report("models: \(error.localizedDescription)")
        }

        /// Resizes the trees, by writing the new size into their data. Before the style has loaded it is only
        /// remembered, for `fillSources`.
        func apply(treeSize: Double) {
            guard treeSize != self.treeSize else { return }
            self.treeSize = treeSize
            (style?.source(withIdentifier: "trees") as? MLNShapeSource)?.shape = City.trees(size: treeSize)
        }

        func stop() {
            timer?.invalidate()
            timer = nil
        }

        private func tick() {
            guard let vehicles = style?.source(withIdentifier: "vehicles") as? MLNShapeSource else { return }
            vehicles.shape = City.vehicles(at: CACurrentMediaTime() - start)
        }
    }
}

/// The demo city's geometry. Its layers are in `models.json`: the roads are drawn `roadWidth` meters wide there, at
/// this latitude, and the cars are scaled up (4 meters per mesh unit) so they are readable. The trees are scaled by
/// their `scale` property, which is the size from the slider.
private enum City {
    static let frame = LocalFrame(origin: CLLocationCoordinate2D(latitude: 52.3731, longitude: 4.8922))

    /// Meters per mesh unit of the trees, and the range of the slider.
    static let defaultTreeSize = 2.0
    static let treeSizes = 1.0 ... 5.0

    static let halfWidth = 55.0
    static let halfHeight = 38.0
    static let roadWidth = 9.0
    static let carSpeed = 11.0 // meters per second
    static let laneHalfLength = 48.0

    /// The ring road, clockwise seen from above.
    static let ring = Loop.roundedRectangle(halfWidth: halfWidth, halfHeight: halfHeight, radius: 14)

    static let cars: [(kind: String, offset: Double)] = [
        ("car-red", 0), ("car-blue", 0.34), ("car-yellow", 0.68),
    ]

    static func vehicles(at time: Double) -> MLNShapeCollectionFeature {
        let features = cars.map { car -> MLNPointFeature in
            let sample = ring.sample(at: car.offset * ring.length + carSpeed * time)
            let feature = MLNPointFeature()
            feature.coordinate = frame.coordinate(east: sample.east, north: sample.north)
            feature.attributes = ["kind": car.kind, "heading": sample.heading]
            return feature
        }
        return MLNShapeCollectionFeature(shapes: features)
    }

    // MARK: Layers

    /// Puts the scene's geometry into the sources `models.json` declares, which its layers draw.
    @MainActor
    static func fillSources(of style: MLNStyle, treeSize: Double, status: DemoStatus) {
        let shapes: [(source: String, shape: MLNShapeCollectionFeature)] = [
            ("roads", roads()), ("buildings", buildings()), ("trees", trees(size: treeSize)),
            ("vehicles", vehicles(at: 0)),
        ]
        for (source, shape) in shapes {
            style.demoSource(source, status: status)?.shape = shape
        }
    }

    private static func roads() -> MLNShapeCollectionFeature {
        func line(_ points: [(east: Double, north: Double)]) -> MLNPolylineFeature {
            var coordinates = points.map { frame.coordinate(east: $0.east, north: $0.north) }
            return MLNPolylineFeature(coordinates: &coordinates, count: UInt(coordinates.count))
        }
        let lane = laneHalfLength + roadWidth / 2
        return MLNShapeCollectionFeature(shapes: [
            line(ring.points.map { ($0.east, $0.north) } + [(ring.points[0].east, ring.points[0].north)]),
            line([(-lane, 0), (lane, 0)]),
            line([(0, -halfHeight), (0, halfHeight)]),
        ])
    }

    /// Four blocks around the crossing lanes and a few more outside the ring.
    private static func buildings() -> MLNShapeCollectionFeature {
        let blocks: [(east: ClosedRange<Double>, north: ClosedRange<Double>, height: Double, color: String)] = [
            (-45 ... -8, 8 ... 28, 34, "#D9CFC1"),
            (8 ... 45, 8 ... 28, 22, "#CBB9A6"),
            (-45 ... -8, -28 ... -8, 14, "#E3D9C8"),
            (8 ... 45, -28 ... -8, 28, "#C9C3B8"),
            (-100 ... -72, -20 ... 20, 40, "#BFC5CC"),
            (72 ... 100, -30 ... 10, 18, "#D4C5B0"),
        ]
        let polygons = blocks.map { block -> MLNPolygonFeature in
            var corners = [
                frame.coordinate(east: block.east.lowerBound, north: block.north.lowerBound),
                frame.coordinate(east: block.east.upperBound, north: block.north.lowerBound),
                frame.coordinate(east: block.east.upperBound, north: block.north.upperBound),
                frame.coordinate(east: block.east.lowerBound, north: block.north.upperBound),
            ]
            let polygon = MLNPolygonFeature(coordinates: &corners, count: UInt(corners.count))
            polygon.attributes = ["height": block.height, "color": block.color]
            return polygon
        }
        return MLNShapeCollectionFeature(shapes: polygons)
    }

    /// Trees along both sides of the ring road, each drawn `size` meters per mesh unit. Where the trees stand doesn't
    /// depend on the size, so resizing only changes their `scale` property.
    static func trees(size: Double) -> MLNShapeCollectionFeature {
        var features: [MLNPointFeature] = []
        var seed: UInt64 = 7
        func random() -> Double {
            seed = seed &* 6_364_136_223_846_793_005 &+ 1_442_695_040_888_963_407
            return Double(seed >> 33) / Double(1 << 31)
        }
        var distance = 0.0
        while distance < ring.length {
            let sample = ring.sample(at: distance)
            let heading = sample.heading * .pi / 180
            let left = (east: -cos(heading), north: sin(heading)) // outside of a clockwise ring
            for side in [-1.0, 1.0] where side > 0 || random() > 0.4 {
                let offset = (side > 0 ? roadWidth / 2 + 4 : -(roadWidth / 2 + 3)) + (random() - 0.5) * 2
                let feature = MLNPointFeature()
                feature.coordinate = frame.coordinate(
                    east: sample.east + left.east * offset, north: sample.north + left.north * offset
                )
                feature.attributes = ["scale": size]
                features.append(feature)
            }
            distance += 14 + random() * 8
        }
        return MLNShapeCollectionFeature(shapes: features)
    }
}

/// A closed path in the local east/north frame, sampled by the distance travelled along it.
struct Loop {
    private(set) var points: [(east: Double, north: Double)]
    private let cumulative: [Double]
    let length: Double

    init(points: [(east: Double, north: Double)]) {
        self.points = points
        var total = 0.0
        var cumulative = [0.0]
        for index in points.indices {
            let next = points[(index + 1) % points.count]
            total += hypot(next.east - points[index].east, next.north - points[index].north)
            cumulative.append(total)
        }
        self.cumulative = cumulative
        length = total
    }

    /// The position and the compass heading (degrees clockwise from north) after `distance` meters, wrapping around.
    func sample(at distance: Double) -> (east: Double, north: Double, heading: Double) {
        var d = distance.truncatingRemainder(dividingBy: length)
        if d < 0 { d += length }
        let index = cumulative.lastIndex(where: { $0 <= d }) ?? 0
        let start = points[index % points.count]
        let end = points[(index + 1) % points.count]
        let segment = cumulative[index + 1] - cumulative[index]
        let t = segment > 0 ? (d - cumulative[index]) / segment : 0
        var heading = atan2(end.east - start.east, end.north - start.north) * 180 / .pi
        if heading < 0 { heading += 360 }
        return (start.east + (end.east - start.east) * t, start.north + (end.north - start.north) * t, heading)
    }

    /// A rectangle with rounded corners, clockwise from above, starting at the west end of the north side.
    static func roundedRectangle(halfWidth a: Double, halfHeight b: Double, radius r: Double) -> Loop {
        var points: [(east: Double, north: Double)] = []
        func arc(centerEast: Double, centerNorth: Double, from start: Double, to end: Double) {
            let steps = 8
            for step in 0 ... steps {
                let angle = (start + (end - start) * Double(step) / Double(steps)) * .pi / 180
                points.append((centerEast + r * cos(angle), centerNorth + r * sin(angle)))
            }
        }
        // Each arc runs clockwise (decreasing angle), from the end of one side to the start of the next.
        arc(centerEast: a - r, centerNorth: b - r, from: 90, to: 0)
        arc(centerEast: a - r, centerNorth: -b + r, from: 0, to: -90)
        arc(centerEast: -a + r, centerNorth: -b + r, from: -90, to: -180)
        arc(centerEast: -a + r, centerNorth: b - r, from: 180, to: 90)
        return Loop(points: points)
    }
}
