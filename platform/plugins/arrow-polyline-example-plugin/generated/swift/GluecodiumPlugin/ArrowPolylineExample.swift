//

//

import Foundation

/// An example plugin that draws arrow polylines on the map.
/// Takes a list of coordinates and draws a polyline with a chevron arrow head.
public class ArrowPolylineExample: MaplibrePlugin {
    override public init() {
        let _result = ArrowPolylineExample.create()
        super.init(cMaplibrePlugin: _result)
        GluecodiumPlugin_ArrowPolylineExample_cache_swift_object_wrapper(c_instance, Unmanaged<AnyObject>.passUnretained(self).toOpaque())
    }

    init(cArrowPolylineExample: _baseRef) {
        super.init(cMaplibrePlugin: cArrowPolylineExample)
    }

    private static func create() -> _baseRef {
        let c_result_handle = GluecodiumPlugin_ArrowPolylineExample_create()
        return moveFromCType(c_result_handle)
    }

    /// Add an arrow polyline to the map
    /// - Parameters:
    ///   - coordinates: List of LatLng coordinates (at least 2 points)
    ///   - config: Arrow appearance configuration
    public func addArrowPolyline(coordinates: [LatLng], config: ArrowPolylineConfig) {
        let c_coordinates = moveToCType(coordinates)
        let c_config = moveToCType(config)
        GluecodiumPlugin_ArrowPolylineExample_addArrowPolyline(c_instance, c_coordinates.ref, c_config.ref)
    }

    /// Remove the current arrow polyline from the map
    public func removeArrowPolyline() {
        GluecodiumPlugin_ArrowPolylineExample_removeArrowPolyline(c_instance)
    }
}

@_cdecl("_CBridgeInitGluecodiumPlugin_ArrowPolylineExample")
func _CBridgeInitGluecodiumPlugin_ArrowPolylineExample(handle: _baseRef) -> UnsafeMutableRawPointer {
    let reference = ArrowPolylineExample(cArrowPolylineExample: handle)
    return Unmanaged<AnyObject>.passRetained(reference).toOpaque()
}

func getRef(_ ref: ArrowPolylineExample?, owning: Bool = true) -> RefHolder {
    guard let c_handle = ref?.c_instance else {
        return RefHolder(0)
    }
    let handle_copy = GluecodiumPlugin_ArrowPolylineExample_copy_handle(c_handle)
    return owning
        ? RefHolder(ref: handle_copy, release: GluecodiumPlugin_ArrowPolylineExample_release_handle)
        : RefHolder(handle_copy)
}

func ArrowPolylineExample_copyFromCType(_ handle: _baseRef) -> ArrowPolylineExample {
    if let swift_pointer = GluecodiumPlugin_ArrowPolylineExample_get_swift_object_from_wrapper_cache(handle),
       let re_constructed = Unmanaged<AnyObject>.fromOpaque(swift_pointer).takeUnretainedValue() as? ArrowPolylineExample
    {
        return re_constructed
    }
    if let swift_pointer = GluecodiumPlugin_ArrowPolylineExample_get_typed(GluecodiumPlugin_ArrowPolylineExample_copy_handle(handle)),
       let typed = Unmanaged<AnyObject>.fromOpaque(swift_pointer).takeRetainedValue() as? ArrowPolylineExample
    {
        GluecodiumPlugin_ArrowPolylineExample_cache_swift_object_wrapper(handle, swift_pointer)
        return typed
    }
    fatalError("Failed to initialize Swift object")
}

func ArrowPolylineExample_moveFromCType(_ handle: _baseRef) -> ArrowPolylineExample {
    if let swift_pointer = GluecodiumPlugin_ArrowPolylineExample_get_swift_object_from_wrapper_cache(handle),
       let re_constructed = Unmanaged<AnyObject>.fromOpaque(swift_pointer).takeUnretainedValue() as? ArrowPolylineExample
    {
        GluecodiumPlugin_ArrowPolylineExample_release_handle(handle)
        return re_constructed
    }
    if let swift_pointer = GluecodiumPlugin_ArrowPolylineExample_get_typed(handle),
       let typed = Unmanaged<AnyObject>.fromOpaque(swift_pointer).takeRetainedValue() as? ArrowPolylineExample
    {
        GluecodiumPlugin_ArrowPolylineExample_cache_swift_object_wrapper(handle, swift_pointer)
        return typed
    }
    fatalError("Failed to initialize Swift object")
}

func ArrowPolylineExample_copyFromCType(_ handle: _baseRef) -> ArrowPolylineExample? {
    guard handle != 0 else {
        return nil
    }
    return ArrowPolylineExample_moveFromCType(handle) as ArrowPolylineExample
}

func ArrowPolylineExample_moveFromCType(_ handle: _baseRef) -> ArrowPolylineExample? {
    guard handle != 0 else {
        return nil
    }
    return ArrowPolylineExample_moveFromCType(handle) as ArrowPolylineExample
}

func copyToCType(_ swiftClass: ArrowPolylineExample) -> RefHolder {
    getRef(swiftClass, owning: false)
}

func moveToCType(_ swiftClass: ArrowPolylineExample) -> RefHolder {
    getRef(swiftClass, owning: true)
}

func copyToCType(_ swiftClass: ArrowPolylineExample?) -> RefHolder {
    getRef(swiftClass, owning: false)
}

func moveToCType(_ swiftClass: ArrowPolylineExample?) -> RefHolder {
    getRef(swiftClass, owning: true)
}
