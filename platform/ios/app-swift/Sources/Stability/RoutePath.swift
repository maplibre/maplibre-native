import CoreLocation

/// Geodesic distances and linear lat/lng interpolation along each segment.
/// Location samples and route trim both use this so they match.
struct RoutePath {
    let coordinates: [CLLocationCoordinate2D]
    /// Distance along the route from the start to coordinate `i`.
    let prefixDistance: [Double]

    var totalDistance: Double { prefixDistance.last ?? 0 }

    init(coordinates: [CLLocationCoordinate2D]) {
        self.coordinates = coordinates
        var prefixDistance = [0.0]
        prefixDistance.reserveCapacity(max(coordinates.count, 1))
        if coordinates.count >= 2 {
            for i in 0 ..< (coordinates.count - 1) {
                prefixDistance.append(prefixDistance[i] + coordinates[i].distance(to: coordinates[i + 1]))
            }
        }
        self.prefixDistance = prefixDistance
    }

    func point(at distance: Double) -> CLLocationCoordinate2D {
        guard let last = coordinates.last else { return CLLocationCoordinate2D() }
        guard coordinates.count >= 2 else { return last }
        if distance <= 0 { return coordinates[0] }
        if distance >= totalDistance { return last }

        let i = vertexIndex(at: distance)
        let next = min(i + 1, coordinates.count - 1)
        let segmentLength = prefixDistance[next] - prefixDistance[i]
        let fraction = segmentLength > 0 ? (distance - prefixDistance[i]) / segmentLength : 0
        return Self.interpolate(from: coordinates[i], to: coordinates[next], fraction: fraction)
    }

    /// Greatest vertex index with `prefixDistance` at most `distance`.
    // Really, no built-in binary search?
    func vertexIndex(at distance: Double) -> Int {
        guard prefixDistance.count > 1 else { return 0 }
        var low = 0
        var high = prefixDistance.count - 1
        while low < high {
            let mid = (low + high + 1) / 2
            if prefixDistance[mid] <= distance {
                low = mid
            } else {
                high = mid - 1
            }
        }
        return low
    }

    /// Simple linear interpolation on lat/lon (no geodesic) intended for short segments only
    static func interpolate(
        from: CLLocationCoordinate2D,
        to: CLLocationCoordinate2D,
        fraction: Double
    ) -> CLLocationCoordinate2D {
        let t = min(1, max(0, fraction)) // no extrapolation
        return CLLocationCoordinate2D(
            latitude: from.latitude + (to.latitude - from.latitude) * t,
            longitude: from.longitude + (to.longitude - from.longitude) * t
        )
    }

    /// Compute the remaining polyline from `distance`, limited to `lookahead` meters.
    /// The first segment is at least `minLeading` meters so the cut does not
    /// collapse to a sub-pixel stub at vertices (which flickers when zoomed in).
    func remainingCoordinates(from distance: Double, lookahead: Double, minLeading: Double) -> [CLLocationCoordinate2D] {
        guard coordinates.count >= 2 else {
            let point = coordinates.last ?? CLLocationCoordinate2D()
            return [point, point]
        }
        if distance >= totalDistance {
            let last = coordinates.last!
            return [last, last]
        }

        let start = point(at: distance)
        let lead = max(minLeading, 0.05)
        let secondDistance = min(distance + lead, totalDistance)
        var remaining = [start]
        if secondDistance > distance {
            remaining.append(point(at: secondDistance))
        }

        var idx = vertexIndex(at: secondDistance) + 1
        let limit = min(distance + max(lookahead, lead), totalDistance)

        while idx < coordinates.count {
            let vertex = coordinates[idx]
            if let last = remaining.last, last.distance(to: vertex) < lead {
                idx += 1
                continue
            }
            remaining.append(vertex)
            if prefixDistance[idx] >= limit, remaining.count >= 2 {
                break
            }
            idx += 1
        }

        if remaining.count < 2 {
            remaining.append(start)
        }
        return remaining
    }
}
