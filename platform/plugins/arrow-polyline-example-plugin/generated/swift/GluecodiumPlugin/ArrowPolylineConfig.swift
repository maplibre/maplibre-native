//

//

import Foundation

/// Configuration for arrow polyline appearance
public struct ArrowPolylineConfig {
    /// Length of the arrow head in pixels

    public var headLength: Double
    /// Angle of the arrow head in degrees

    public var headAngle: Double
    /// Color of the arrow line as hex string (e.g., "#FF0000")

    public var lineColor: String
    /// Width of the arrow line in pixels

    public var lineWidth: Double

    public init(headLength: Double = 50.0, headAngle: Double = 30.0, lineColor: String = "#FF0000", lineWidth: Double = 3.0) {
        self.headLength = headLength
        self.headAngle = headAngle
        self.lineColor = lineColor
        self.lineWidth = lineWidth
    }

    init(cHandle: _baseRef) {
        headLength = moveFromCType(GluecodiumPlugin_ArrowPolylineConfig_headLength_get(cHandle))
        headAngle = moveFromCType(GluecodiumPlugin_ArrowPolylineConfig_headAngle_get(cHandle))
        lineColor = moveFromCType(GluecodiumPlugin_ArrowPolylineConfig_lineColor_get(cHandle))
        lineWidth = moveFromCType(GluecodiumPlugin_ArrowPolylineConfig_lineWidth_get(cHandle))
    }
}

func copyFromCType(_ handle: _baseRef) -> ArrowPolylineConfig {
    ArrowPolylineConfig(cHandle: handle)
}

func moveFromCType(_ handle: _baseRef) -> ArrowPolylineConfig {
    defer {
        GluecodiumPlugin_ArrowPolylineConfig_release_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: ArrowPolylineConfig) -> RefHolder {
    let c_headLength = moveToCType(swiftType.headLength)
    let c_headAngle = moveToCType(swiftType.headAngle)
    let c_lineColor = moveToCType(swiftType.lineColor)
    let c_lineWidth = moveToCType(swiftType.lineWidth)
    return RefHolder(GluecodiumPlugin_ArrowPolylineConfig_create_handle(c_headLength.ref, c_headAngle.ref, c_lineColor.ref, c_lineWidth.ref))
}

func moveToCType(_ swiftType: ArrowPolylineConfig) -> RefHolder {
    RefHolder(ref: copyToCType(swiftType).ref, release: GluecodiumPlugin_ArrowPolylineConfig_release_handle)
}

func copyFromCType(_ handle: _baseRef) -> ArrowPolylineConfig? {
    guard handle != 0 else {
        return nil
    }
    let unwrappedHandle = GluecodiumPlugin_ArrowPolylineConfig_unwrap_optional_handle(handle)
    return ArrowPolylineConfig(cHandle: unwrappedHandle) as ArrowPolylineConfig
}

func moveFromCType(_ handle: _baseRef) -> ArrowPolylineConfig? {
    defer {
        GluecodiumPlugin_ArrowPolylineConfig_release_optional_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: ArrowPolylineConfig?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    let c_headLength = moveToCType(swiftType.headLength)
    let c_headAngle = moveToCType(swiftType.headAngle)
    let c_lineColor = moveToCType(swiftType.lineColor)
    let c_lineWidth = moveToCType(swiftType.lineWidth)
    return RefHolder(GluecodiumPlugin_ArrowPolylineConfig_create_optional_handle(c_headLength.ref, c_headAngle.ref, c_lineColor.ref, c_lineWidth.ref))
}

func moveToCType(_ swiftType: ArrowPolylineConfig?) -> RefHolder {
    RefHolder(ref: copyToCType(swiftType).ref, release: GluecodiumPlugin_ArrowPolylineConfig_release_optional_handle)
}
