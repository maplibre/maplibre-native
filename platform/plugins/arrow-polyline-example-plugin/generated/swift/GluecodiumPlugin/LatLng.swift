//

//

import Foundation

/// A geographic coordinate with latitude and longitude
public struct LatLng: Hashable {
    public var latitude: Double

    public var longitude: Double

    public init(latitude: Double, longitude: Double) {
        self.latitude = latitude
        self.longitude = longitude
    }

    init(cHandle: _baseRef) {
        latitude = moveFromCType(GluecodiumPlugin_LatLng_latitude_get(cHandle))
        longitude = moveFromCType(GluecodiumPlugin_LatLng_longitude_get(cHandle))
    }
}

func copyFromCType(_ handle: _baseRef) -> LatLng {
    LatLng(cHandle: handle)
}

func moveFromCType(_ handle: _baseRef) -> LatLng {
    defer {
        GluecodiumPlugin_LatLng_release_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: LatLng) -> RefHolder {
    let c_latitude = moveToCType(swiftType.latitude)
    let c_longitude = moveToCType(swiftType.longitude)
    return RefHolder(GluecodiumPlugin_LatLng_create_handle(c_latitude.ref, c_longitude.ref))
}

func moveToCType(_ swiftType: LatLng) -> RefHolder {
    RefHolder(ref: copyToCType(swiftType).ref, release: GluecodiumPlugin_LatLng_release_handle)
}

func copyFromCType(_ handle: _baseRef) -> LatLng? {
    guard handle != 0 else {
        return nil
    }
    let unwrappedHandle = GluecodiumPlugin_LatLng_unwrap_optional_handle(handle)
    return LatLng(cHandle: unwrappedHandle) as LatLng
}

func moveFromCType(_ handle: _baseRef) -> LatLng? {
    defer {
        GluecodiumPlugin_LatLng_release_optional_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: LatLng?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    let c_latitude = moveToCType(swiftType.latitude)
    let c_longitude = moveToCType(swiftType.longitude)
    return RefHolder(GluecodiumPlugin_LatLng_create_optional_handle(c_latitude.ref, c_longitude.ref))
}

func moveToCType(_ swiftType: LatLng?) -> RefHolder {
    RefHolder(ref: copyToCType(swiftType).ref, release: GluecodiumPlugin_LatLng_release_optional_handle)
}
