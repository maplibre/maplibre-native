//

//

import Foundation

func copyFromCType(_ handle: _baseRef) -> [LatLng] {
    var result: [LatLng] = []
    let count = ArrayOf_GluecodiumPlugin_LatLng_count(handle)
    for idx in 0 ..< count {
        result.append(copyFromCType(ArrayOf_GluecodiumPlugin_LatLng_get(handle, idx)))
    }
    return result
}

func moveFromCType(_ handle: _baseRef) -> [LatLng] {
    defer {
        ArrayOf_GluecodiumPlugin_LatLng_release_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftArray: [LatLng]) -> RefHolder {
    let handle = ArrayOf_GluecodiumPlugin_LatLng_create_handle()
    for item in swiftArray {
        let _item = moveToCType(item)
        ArrayOf_GluecodiumPlugin_LatLng_append(handle, _item.ref)
    }
    return RefHolder(handle)
}

func moveToCType(_ swiftArray: [LatLng]) -> RefHolder {
    RefHolder(ref: copyToCType(swiftArray).ref, release: ArrayOf_GluecodiumPlugin_LatLng_release_handle)
}

func copyToCType(_ swiftArray: [LatLng]?) -> RefHolder {
    guard let swiftArray else {
        return RefHolder(0)
    }
    let optionalHandle = ArrayOf_GluecodiumPlugin_LatLng_create_optional_handle()
    let handle = ArrayOf_GluecodiumPlugin_LatLng_unwrap_optional_handle(optionalHandle)
    for item in swiftArray {
        let _item = moveToCType(item)
        ArrayOf_GluecodiumPlugin_LatLng_append(handle, _item.ref)
    }
    return RefHolder(optionalHandle)
}

func moveToCType(_ swiftType: [LatLng]?) -> RefHolder {
    RefHolder(ref: copyToCType(swiftType).ref, release: ArrayOf_GluecodiumPlugin_LatLng_release_optional_handle)
}

func copyFromCType(_ handle: _baseRef) -> [LatLng]? {
    guard handle != 0 else {
        return nil
    }
    let unwrappedHandle = ArrayOf_GluecodiumPlugin_LatLng_unwrap_optional_handle(handle)
    return copyFromCType(unwrappedHandle) as [LatLng]
}

func moveFromCType(_ handle: _baseRef) -> [LatLng]? {
    defer {
        ArrayOf_GluecodiumPlugin_LatLng_release_optional_handle(handle)
    }
    return copyFromCType(handle)
}
