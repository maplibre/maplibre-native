import Mapbox
import MapKit


public struct CoordinateBounds {
    /// Internally used MKMapRect used for comparison
    var mapRect: MKMapRect {
        return MKPolyline(coordinates: [self.northEast, self.southWest], count: 2).boundingMapRect
    }

    /// The North-East corner of these bounds.
    public let northEast: CLLocationCoordinate2D

    /// The South-West corner of these bounds.
    public let southWest: CLLocationCoordinate2D


    /// Creates a CoordinateBounds containing every point included in the passed-in array. Note that this
    /// method doesn't work well if you are crossing the north pole or crossing the 180th meridian.
    ///
    /// - parameter coordinates: An array of coordinates from which the bounds will be calculated
    public init(including coordinates: [CLLocationCoordinate2D]) {

        let mapRect = MKPolyline(coordinates: coordinates, count: coordinates.count).boundingMapRect
        self.northEast = MKMapPoint(x: mapRect.maxX, y: mapRect.origin.y).coordinate
        self.southWest = MKMapPoint(x: mapRect.origin.x, y: mapRect.maxY).coordinate
    }

    /// Creates a CoordinateBounds containing every point from given list of CoordinateBounds.
    ///
    /// - parameter boundsList: An array of CoordinateBounds from which the bounds will be calculated
    public init?(combining boundsList: [CoordinateBounds]) {
        guard let first = boundsList.first else {
            return nil
        }

        self = boundsList[1...].reduce(first) { $0.includingBounds(bounds: $1) }
    }

    /// Inits the northEast and southWest bounds corresponding to the rectangular region defined by the two
    /// corners.
    ///
    /// It is ambiguous whether the longitude of the box extends from `coord1` to `coord2` or vice-versa,
    /// the box is constructed as the smaller of the two variants, eliminating the ambiguity.
    ///
    /// - parameter coord1: One of the two corners of the box
    /// - parameter coord2: The other corners of the box
    public init(coordinate coord1: CLLocationCoordinate2D, coordinate coord2: CLLocationCoordinate2D) {
        self.init(including: [coord1, coord2])
    }

    /// Returns a CoordinateBounds representing the current bounds extended to include the entire other
    /// bounds.
    ///
    /// - parameter bounds: The bounds used to extend the receiver.
    ///
    /// - returns: the newly extended bounds.
    public func includingBounds(bounds: CoordinateBounds) -> CoordinateBounds {
        return CoordinateBounds(including: [
            self.northEast, self.southWest, bounds.northEast, bounds.southWest,
        ])
    }

    /// Returns true if `coordinate` is contained within this bounds. This includes points that lie
    /// exactly on the edge of the bounds.
    ///
    /// - parameter coordinate: The coordinate to check if it lies inside the bounds.
    ///
    /// - returns: A boolean indicating if the point is cointained inside the receiver bounds.
    public func contains(_ coordinate: CLLocationCoordinate2D) -> Bool {
        return self.mapRect.contains(MKMapPoint(coordinate))
    }

    /// Returns a CoordinateBounds representing the current bounds extended to include the passed-in
    /// coordinates.
    ///
    /// - parameter coordinates: An array of coordinates to include into the bounds.
    ///
    /// - returns: the newly extended bounds.
    public func including(coordinates: [CLLocationCoordinate2D]) -> CoordinateBounds {
        var newCoordinates = coordinates
        newCoordinates.append(contentsOf: [self.northEast, self.southWest])
        return CoordinateBounds(including: newCoordinates)
    }

    /// Returns true if `other` overlaps with this bounds.
    ///
    /// - parameter bounds: The other bounds to check if it intersects `self`.
    ///
    /// - returns: A boolean indicating if the two bounds intersect.
    public func intersects(_ bounds: CoordinateBounds) -> Bool {
        return self.mapRect.intersects(bounds.mapRect)
    }
}


extension CoordinateBounds {
    /// Mapbox coordinate bounds conversion
    var mbBounds: MGLCoordinateBounds {
        return MGLCoordinateBounds(sw: self.southWest, ne: self.northEast)
    }

    /// Init
    ///
    /// - parameter bounds: mapbox bounds used to initialize.
    init(_ bounds: MGLCoordinateBounds) {
        self.init(including: [bounds.ne, bounds.sw])
    }
}
