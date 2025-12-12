//

//

func copyToCType(_ swiftType: Bool?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(bool_create_handle(swiftType))
}

func moveToCType(_ swiftType: Bool?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(ref: bool_create_handle(swiftType), release: bool_release_handle)
}

func copyFromCType(_ handle: _baseRef) -> Bool? {
    guard handle != 0 else {
        return nil
    }
    return bool_value_get(handle)
}

func moveFromCType(_ handle: _baseRef) -> Bool? {
    defer {
        bool_release_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: Float?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(float_create_handle(swiftType))
}

func moveToCType(_ swiftType: Float?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(ref: float_create_handle(swiftType), release: float_release_handle)
}

func copyFromCType(_ handle: _baseRef) -> Float? {
    guard handle != 0 else {
        return nil
    }
    return float_value_get(handle)
}

func moveFromCType(_ handle: _baseRef) -> Float? {
    defer {
        float_release_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: Double?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(double_create_handle(swiftType))
}

func moveToCType(_ swiftType: Double?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(ref: double_create_handle(swiftType), release: double_release_handle)
}

func copyFromCType(_ handle: _baseRef) -> Double? {
    guard handle != 0 else {
        return nil
    }
    return double_value_get(handle)
}

func moveFromCType(_ handle: _baseRef) -> Double? {
    defer {
        double_release_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: Int8?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(int8_t_create_handle(swiftType))
}

func moveToCType(_ swiftType: Int8?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(ref: int8_t_create_handle(swiftType), release: int8_t_release_handle)
}

func copyFromCType(_ handle: _baseRef) -> Int8? {
    guard handle != 0 else {
        return nil
    }
    return int8_t_value_get(handle)
}

func moveFromCType(_ handle: _baseRef) -> Int8? {
    defer {
        int8_t_release_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: UInt8?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(uint8_t_create_handle(swiftType))
}

func moveToCType(_ swiftType: UInt8?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(ref: uint8_t_create_handle(swiftType), release: uint8_t_release_handle)
}

func copyFromCType(_ handle: _baseRef) -> UInt8? {
    guard handle != 0 else {
        return nil
    }
    return uint8_t_value_get(handle)
}

func moveFromCType(_ handle: _baseRef) -> UInt8? {
    defer {
        uint8_t_release_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: Int16?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(int16_t_create_handle(swiftType))
}

func moveToCType(_ swiftType: Int16?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(ref: int16_t_create_handle(swiftType), release: int16_t_release_handle)
}

func copyFromCType(_ handle: _baseRef) -> Int16? {
    guard handle != 0 else {
        return nil
    }
    return int16_t_value_get(handle)
}

func moveFromCType(_ handle: _baseRef) -> Int16? {
    defer {
        int16_t_release_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: UInt16?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(uint16_t_create_handle(swiftType))
}

func moveToCType(_ swiftType: UInt16?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(ref: uint16_t_create_handle(swiftType), release: uint16_t_release_handle)
}

func copyFromCType(_ handle: _baseRef) -> UInt16? {
    guard handle != 0 else {
        return nil
    }
    return uint16_t_value_get(handle)
}

func moveFromCType(_ handle: _baseRef) -> UInt16? {
    defer {
        uint16_t_release_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: Int32?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(int32_t_create_handle(swiftType))
}

func moveToCType(_ swiftType: Int32?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(ref: int32_t_create_handle(swiftType), release: int32_t_release_handle)
}

func copyFromCType(_ handle: _baseRef) -> Int32? {
    guard handle != 0 else {
        return nil
    }
    return int32_t_value_get(handle)
}

func moveFromCType(_ handle: _baseRef) -> Int32? {
    defer {
        int32_t_release_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: UInt32?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(uint32_t_create_handle(swiftType))
}

func moveToCType(_ swiftType: UInt32?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(ref: uint32_t_create_handle(swiftType), release: uint32_t_release_handle)
}

func copyFromCType(_ handle: _baseRef) -> UInt32? {
    guard handle != 0 else {
        return nil
    }
    return uint32_t_value_get(handle)
}

func moveFromCType(_ handle: _baseRef) -> UInt32? {
    defer {
        uint32_t_release_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: Int64?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(int64_t_create_handle(swiftType))
}

func moveToCType(_ swiftType: Int64?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(ref: int64_t_create_handle(swiftType), release: int64_t_release_handle)
}

func copyFromCType(_ handle: _baseRef) -> Int64? {
    guard handle != 0 else {
        return nil
    }
    return int64_t_value_get(handle)
}

func moveFromCType(_ handle: _baseRef) -> Int64? {
    defer {
        int64_t_release_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: UInt64?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(uint64_t_create_handle(swiftType))
}

func moveToCType(_ swiftType: UInt64?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(ref: uint64_t_create_handle(swiftType), release: uint64_t_release_handle)
}

func copyFromCType(_ handle: _baseRef) -> UInt64? {
    guard handle != 0 else {
        return nil
    }
    return uint64_t_value_get(handle)
}

func moveFromCType(_ handle: _baseRef) -> UInt64? {
    defer {
        uint64_t_release_handle(handle)
    }
    return copyFromCType(handle)
}
