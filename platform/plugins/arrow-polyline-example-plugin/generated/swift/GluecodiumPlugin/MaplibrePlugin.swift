//

//

import Foundation

/// Base class for gluecodium-based maplibre plugins
/// (In a full integration, this would be in a shared package)
public class MaplibrePlugin {
    public init() {
        let _result = MaplibrePlugin.create()
        guard _result != 0 else {
            fatalError("Nullptr value is not supported for initializers")
        }
        c_instance = _result
        GluecodiumPlugin_MaplibrePlugin_cache_swift_object_wrapper(c_instance, Unmanaged<AnyObject>.passUnretained(self).toOpaque())
    }

    /// Pointer to the underlying C++ class that implements XPlatformPlugin

    public var ptr: UInt64 {
        let c_result_handle = GluecodiumPlugin_MaplibrePlugin_ptr_get(c_instance)
        return moveFromCType(c_result_handle)
    }

    let c_instance: _baseRef

    init(cMaplibrePlugin: _baseRef) {
        guard cMaplibrePlugin != 0 else {
            fatalError("Nullptr value is not supported for initializers")
        }
        c_instance = cMaplibrePlugin
    }

    deinit {
        GluecodiumPlugin_MaplibrePlugin_remove_swift_object_from_wrapper_cache(c_instance)
        GluecodiumPlugin_MaplibrePlugin_release_handle(c_instance)
    }

    private static func create() -> _baseRef {
        let c_result_handle = GluecodiumPlugin_MaplibrePlugin_create()
        return moveFromCType(c_result_handle)
    }
}

@_cdecl("_CBridgeInitGluecodiumPlugin_MaplibrePlugin")
func _CBridgeInitGluecodiumPlugin_MaplibrePlugin(handle: _baseRef) -> UnsafeMutableRawPointer {
    let reference = MaplibrePlugin(cMaplibrePlugin: handle)
    return Unmanaged<AnyObject>.passRetained(reference).toOpaque()
}

func getRef(_ ref: MaplibrePlugin?, owning: Bool = true) -> RefHolder {
    guard let c_handle = ref?.c_instance else {
        return RefHolder(0)
    }
    let handle_copy = GluecodiumPlugin_MaplibrePlugin_copy_handle(c_handle)
    return owning
        ? RefHolder(ref: handle_copy, release: GluecodiumPlugin_MaplibrePlugin_release_handle)
        : RefHolder(handle_copy)
}

extension MaplibrePlugin: NativeBase {
    /// :nodoc:
    var c_handle: _baseRef { c_instance }
}

extension MaplibrePlugin: Hashable {
    /// :nodoc:
    public static func == (lhs: MaplibrePlugin, rhs: MaplibrePlugin) -> Bool {
        lhs.c_handle == rhs.c_handle
    }

    /// :nodoc:
    public func hash(into hasher: inout Hasher) {
        hasher.combine(c_handle)
    }
}

func MaplibrePlugin_copyFromCType(_ handle: _baseRef) -> MaplibrePlugin {
    if let swift_pointer = GluecodiumPlugin_MaplibrePlugin_get_swift_object_from_wrapper_cache(handle),
       let re_constructed = Unmanaged<AnyObject>.fromOpaque(swift_pointer).takeUnretainedValue() as? MaplibrePlugin
    {
        return re_constructed
    }
    if let swift_pointer = GluecodiumPlugin_MaplibrePlugin_get_typed(GluecodiumPlugin_MaplibrePlugin_copy_handle(handle)),
       let typed = Unmanaged<AnyObject>.fromOpaque(swift_pointer).takeRetainedValue() as? MaplibrePlugin
    {
        GluecodiumPlugin_MaplibrePlugin_cache_swift_object_wrapper(handle, swift_pointer)
        return typed
    }
    fatalError("Failed to initialize Swift object")
}

func MaplibrePlugin_moveFromCType(_ handle: _baseRef) -> MaplibrePlugin {
    if let swift_pointer = GluecodiumPlugin_MaplibrePlugin_get_swift_object_from_wrapper_cache(handle),
       let re_constructed = Unmanaged<AnyObject>.fromOpaque(swift_pointer).takeUnretainedValue() as? MaplibrePlugin
    {
        GluecodiumPlugin_MaplibrePlugin_release_handle(handle)
        return re_constructed
    }
    if let swift_pointer = GluecodiumPlugin_MaplibrePlugin_get_typed(handle),
       let typed = Unmanaged<AnyObject>.fromOpaque(swift_pointer).takeRetainedValue() as? MaplibrePlugin
    {
        GluecodiumPlugin_MaplibrePlugin_cache_swift_object_wrapper(handle, swift_pointer)
        return typed
    }
    fatalError("Failed to initialize Swift object")
}

func MaplibrePlugin_copyFromCType(_ handle: _baseRef) -> MaplibrePlugin? {
    guard handle != 0 else {
        return nil
    }
    return MaplibrePlugin_moveFromCType(handle) as MaplibrePlugin
}

func MaplibrePlugin_moveFromCType(_ handle: _baseRef) -> MaplibrePlugin? {
    guard handle != 0 else {
        return nil
    }
    return MaplibrePlugin_moveFromCType(handle) as MaplibrePlugin
}

func copyToCType(_ swiftClass: MaplibrePlugin) -> RefHolder {
    getRef(swiftClass, owning: false)
}

func moveToCType(_ swiftClass: MaplibrePlugin) -> RefHolder {
    getRef(swiftClass, owning: true)
}

func copyToCType(_ swiftClass: MaplibrePlugin?) -> RefHolder {
    getRef(swiftClass, owning: false)
}

func moveToCType(_ swiftClass: MaplibrePlugin?) -> RefHolder {
    getRef(swiftClass, owning: true)
}
