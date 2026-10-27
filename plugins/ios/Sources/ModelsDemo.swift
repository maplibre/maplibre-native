import MapLibre
import SwiftUI

/// The `gltf` layer: `.glb` models drawn at the points of a source. A feature picks its model with `model-id`, and
/// heading, scale, altitude and opacity are paint properties evaluated every frame.
///
/// The scene is a ring road with three cars on it, trees, and a few extruded buildings. A user "puck" drives up and
/// down the lane between the buildings. The cars and trees are depth tested against the buildings (`model-occlusion`),
/// the puck is drawn over everything.
struct ModelsDemoView: View {
    @StateObject private var status = DemoStatus()
    @State private var driving = true
    @State private var occlusion = true
    @State private var exaggeration = 4.0

    var body: some View {
        ZStack(alignment: .top) {
            ModelsMap(driving: driving, occlusion: occlusion, exaggeration: exaggeration, status: status)
            StatusBanner(status: status)
        }
        .safeAreaInset(edge: .bottom, spacing: 0) {
            ControlPanel(
                title: "gltf layer",
                detail: "Each car feature names its model with model-id and its heading with model-heading. "
                    + "Models are sized in meters, exaggerated here so they are readable."
            ) {
                Toggle("Drive", isOn: $driving)
                Toggle("Buildings hide the cars and trees", isOn: $occlusion)
                HStack {
                    Text("Size").frame(width: 50, alignment: .leading)
                    Slider(value: $exaggeration, in: 1 ... 5)
                    Text("\(exaggeration, specifier: "%.1f")×").monospacedDigit().frame(width: 40)
                }
            }
        }
    }
}

private struct ModelsMap: UIViewRepresentable {
    var driving: Bool
    var occlusion: Bool
    var exaggeration: Double
    var status: DemoStatus

    func makeCoordinator() -> Coordinator { Coordinator(status: status) }

    func makeUIView(context: Context) -> MLNMapView {
        let mapView = MLNMapView.demoMap(style: "models")
        mapView.delegate = context.coordinator
        return mapView
    }

    func updateUIView(_: MLNMapView, context: Context) {
        context.coordinator.apply(driving: driving, occlusion: occlusion, exaggeration: exaggeration)
    }

    static func dismantleUIView(_: MLNMapView, coordinator: Coordinator) {
        coordinator.stop()
    }

    @MainActor
    final class Coordinator: NSObject, @preconcurrency MLNMapViewDelegate {
        let status: DemoStatus
        private var style: MLNStyle?
        private var timer: Timer?
        private var lastTick = CACurrentMediaTime()
        private var elapsed = 0.0
        private var driving = true
        private var occlusion = true
        private var exaggeration = 4.0

        init(status: DemoStatus) { self.status = status }

        func mapView(_ mapView: MLNMapView, didFinishLoading style: MLNStyle) {
            self.style = style
            mapView.look(at: City.frame.origin, zoom: 16.9, pitch: 58, heading: 12)
            City.addLayers(to: style, exaggeration: exaggeration, status: status)
            apply(driving: driving, occlusion: occlusion, exaggeration: exaggeration)
            lastTick = CACurrentMediaTime()
            timer?.invalidate()
            timer = Timer.scheduledTimer(withTimeInterval: 1.0 / 20, repeats: true) { [weak self] _ in
                MainActor.assumeIsolated { self?.tick() }
            }
            tick()
        }

        func stop() {
            timer?.invalidate()
            timer = nil
        }

        func apply(driving: Bool, occlusion: Bool, exaggeration: Double) {
            self.driving = driving
            self.exaggeration = exaggeration
            let occlusionChanged = self.occlusion != occlusion
            self.occlusion = occlusion
            guard let style else { return }
            // `model-scale` is a paint property, so this only repaints.
            style.set("model-scale", exaggeration, on: "vehicles", status: status)
            style.set("model-scale", ["*", exaggeration, ["coalesce", ["get", "scale"], 1]], on: "trees", status: status)
            // `model-occlusion` is a layout property, which lays the tiles out again.
            if occlusionChanged {
                for layer in ["vehicles", "trees"] {
                    style.set("model-occlusion", occlusion ? 1 : 0, on: layer, status: status)
                }
            }
        }

        private func tick() {
            let now = CACurrentMediaTime()
            defer { lastTick = now }
            guard driving, let style else { return }
            elapsed += now - lastTick
            if let vehicles = style.source(withIdentifier: "vehicles") as? MLNShapeSource {
                vehicles.shape = City.vehicles(at: elapsed)
            }
            if let puck = style.source(withIdentifier: "puck") as? MLNShapeSource {
                puck.shape = City.puck(at: elapsed)
            }
        }
    }
}

/// The demo city's geometry and its layers.
private enum City {
    static let frame = LocalFrame(origin: CLLocationCoordinate2D(latitude: 52.3731, longitude: 4.8922))

    static let halfWidth = 55.0
    static let halfHeight = 38.0
    static let roadWidth = 9.0
    static let carSpeed = 11.0 // meters per second
    static let puckSpeed = 7.0
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

    /// The puck drives east along the central lane and back.
    static func puck(at time: Double) -> MLNShapeCollectionFeature {
        let length = 2 * laneHalfLength
        let travelled = (puckSpeed * time).truncatingRemainder(dividingBy: 2 * length)
        let outbound = travelled <= length
        let east = outbound ? travelled - laneHalfLength : 3 * laneHalfLength - travelled
        let feature = MLNPointFeature()
        feature.coordinate = frame.coordinate(east: east, north: 0)
        feature.attributes = ["heading": outbound ? 90 : 270]
        return MLNShapeCollectionFeature(shapes: [feature])
    }

    // MARK: Layers

    @MainActor
    static func addLayers(to style: MLNStyle, exaggeration: Double, status: DemoStatus) {
        style.addSource(MLNShapeSource(identifier: "roads", shape: roads(), options: nil))
        style.addSource(MLNShapeSource(identifier: "buildings", shape: buildings(), options: nil))
        style.addSource(MLNShapeSource(identifier: "trees", shape: trees(), options: nil))
        style.addSource(MLNShapeSource(identifier: "vehicles", shape: vehicles(at: 0), options: nil))
        style.addSource(MLNShapeSource(identifier: "puck", shape: puck(at: 0), options: nil))

        let layers: [[String: Any]] = [
            road(id: "road-casing", color: "#9C9C96", width: roadWidth + 2),
            road(id: "road", color: "#5E6064", width: roadWidth),
            [
                "id": "buildings",
                "type": "fill-extrusion",
                "source": "buildings",
                "paint": [
                    "fill-extrusion-color": ["get", "color"],
                    "fill-extrusion-height": ["get", "height"],
                    "fill-extrusion-opacity": 1,
                ],
            ],
            // One model for every feature, picked by a constant. Each tree has its own size.
            [
                "id": "trees",
                "type": "gltf",
                "source": "trees",
                "layout": ["models": ["tree": "asset://tree.glb"], "model-id": "tree"],
                "paint": ["model-scale": ["*", exaggeration, ["coalesce", ["get", "scale"], 1]]],
            ],
            // Several models in one layer: each car's `kind` property names its model.
            [
                "id": "vehicles",
                "type": "gltf",
                "source": "vehicles",
                "layout": [
                    "models": [
                        "car-red": "asset://car-red.glb",
                        "car-blue": "asset://car-blue.glb",
                        "car-yellow": "asset://car-yellow.glb",
                    ],
                    "model-id": ["get", "kind"],
                ],
                "paint": ["model-heading": ["get", "heading"], "model-scale": exaggeration],
            ],
            // A puck keeps its size on screen (`model-screen-scale` is points per mesh unit), and is drawn over
            // everything below it in the layer order, buildings included.
            [
                "id": "puck",
                "type": "gltf",
                "source": "puck",
                "layout": ["models": ["puck": "asset://puck.glb"], "model-id": "puck", "model-occlusion": 0],
                "paint": ["model-heading": ["get", "heading"], "model-screen-scale": 36],
            ],
        ]
        for layer in layers {
            style.add(layer: layer, status: status)
        }
    }

    /// A line `meters` wide at the map's latitude, whatever the zoom: pixels double with every zoom level.
    private static func road(id: String, color: String, width meters: Double) -> [String: Any] {
        let metersPerPointAtZoom0 = 78271.517 * cos(frame.origin.latitude * .pi / 180)
        let pointsAtZoom10 = meters * 1024 / metersPerPointAtZoom0
        return [
            "id": id,
            "type": "line",
            "source": "roads",
            "layout": ["line-cap": "round", "line-join": "round"],
            "paint": [
                "line-color": color,
                "line-width": ["interpolate", ["exponential", 2], ["zoom"], 10, pointsAtZoom10, 22, pointsAtZoom10 * 4096],
            ],
        ]
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

    /// Trees along both sides of the ring road, at slightly different sizes.
    private static func trees() -> MLNShapeCollectionFeature {
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
                feature.attributes = ["scale": 0.25 + random() * 0.2]
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
