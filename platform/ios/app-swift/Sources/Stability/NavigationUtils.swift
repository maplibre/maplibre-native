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

// TODO: pull swift_polyline or use function?
// https://github.com/raphaelmor/Polyline
public class Geometry {
    public static func decodePolyline(from encodedGeo: String, precision: Double = 1e6) -> [CLLocationCoordinate2D]? {
        guard let rawGeo = encodedGeo.data(using: .utf8) else { return nil }

        return rawGeo.withUnsafeBytes { bytes -> [CLLocationCoordinate2D]? in
            let length = rawGeo.count
            var position = 0

            var decodedCoordinates = [CLLocationCoordinate2D]()

            var lat = 0.0
            var lon = 0.0

            while position < length {
                let resultingLat = decodeSingleCoordinate(byteArray: bytes, length: length, position: &position, precision: precision)
                lat += resultingLat

                let resultingLon = decodeSingleCoordinate(byteArray: bytes, length: length, position: &position, precision: precision)
                lon += resultingLon

                decodedCoordinates.append(CLLocationCoordinate2D(latitude: lat, longitude: lon))
            }

            return decodedCoordinates
        }
    }

    public static func decodeSingleCoordinate(byteArray: UnsafeRawBufferPointer, length: Int, position: inout Int, precision: Double = 1e5) -> Double {
        guard position < length else { return 0.0 }

        let bitMask = Int8(0x1F)

        var coordinate: Int32 = 0

        var currentChar: Int8
        var componentCounter: Int32 = 0
        var component: Int32 = 0

        repeat {
            currentChar = Int8(byteArray[position]) - 63
            component = Int32(currentChar & bitMask)
            coordinate |= (component << (5 * componentCounter))
            position += 1
            componentCounter += 1
        } while ((currentChar & 0x20) == 0x20) && (position < length) && (componentCounter < 6)

        if componentCounter == 6, (currentChar & 0x20) == 0x20 {
            return 0.0
        }

        if (coordinate & 0x01) == 0x01 {
            coordinate = ~(coordinate >> 1)
        } else {
            coordinate = coordinate >> 1
        }

        return Double(coordinate) / precision
    }
}
