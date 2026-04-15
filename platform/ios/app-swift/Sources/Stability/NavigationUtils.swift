import CoreLocation

public extension CLLocationDegrees {
    func toRadians() -> CLLocationDegrees {
        self * .pi / 180.0
    }

    func toDegrees() -> CLLocationDegrees {
        self * 180.0 / .pi
    }
}

public extension CLLocationCoordinate2D {
    func distance(to coordinate: CLLocationCoordinate2D) -> CLLocationDistance {
        let metersPerRadian = 6_373_000.0
        let A = toRadians()
        let B = coordinate.toRadians()

        let a = pow(sin((B.latitude - A.latitude) / 2), 2)
            + pow(sin((B.longitude - A.longitude) / 2), 2) * cos(A.latitude) * cos(B.latitude)
        return 2 * atan2(sqrt(a), sqrt(1 - a)) * metersPerRadian
    }

    func direction(to coordinate: CLLocationCoordinate2D) -> CLLocationDegrees {
        let A = toRadians()
        let B = coordinate.toRadians()

        let a = sin(B.longitude - A.longitude) * cos(B.latitude)
        let b = cos(A.latitude) * sin(B.latitude) - sin(A.latitude) * cos(B.latitude) * cos(B.longitude - A.longitude)
        return Measurement(value: atan2(a, b), unit: UnitAngle.radians).converted(to: .degrees).value
    }

    func coordinate(at distance: CLLocationDistance, facing direction: CLLocationDirection) -> CLLocationCoordinate2D {
        let metersPerRadian = 6_373_000.0
        let distance = distance / metersPerRadian
        let direction = Measurement(value: direction, unit: UnitAngle.degrees).converted(to: .radians).value
        let coordRadians = toRadians()

        let otherLatitude = asin(sin(coordRadians.latitude) * cos(distance) + cos(coordRadians.latitude) * sin(distance) * cos(direction))
        let otherLongitude = coordRadians.longitude + atan2(sin(direction) * sin(distance) * cos(coordRadians.latitude),
                                                            cos(distance) - sin(coordRadians.latitude) * sin(otherLatitude))
        return CLLocationCoordinate2D(latitude: otherLatitude, longitude: otherLongitude).toDegrees()
    }

    func toRadians() -> CLLocationCoordinate2D {
        CLLocationCoordinate2D(latitude: latitude.toRadians(), longitude: longitude.toRadians())
    }

    func toDegrees() -> CLLocationCoordinate2D {
        CLLocationCoordinate2D(latitude: latitude.toDegrees(), longitude: longitude.toDegrees())
    }
}

public extension CLLocationDirection {
    func wrap(min minimumValue: CLLocationDirection, max maximumValue: CLLocationDirection) -> CLLocationDirection {
        let d = maximumValue - minimumValue
        return fmod(fmod(self - minimumValue, d) + d, d) + minimumValue
    }
}
