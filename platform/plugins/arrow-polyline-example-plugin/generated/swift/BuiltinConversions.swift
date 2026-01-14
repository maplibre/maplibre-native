// -------------------------------------------------------------------------------------------------
// Copyright (C) 2016-2019 HERE Europe B.V.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0
// License-Filename: LICENSE
//
// -------------------------------------------------------------------------------------------------

import Foundation

extension String {
    func convertToCType() -> _baseRef {
        let result = std_string_create_handle(self)
        precondition(result != 0, "Out of memory")
        return result
    }
}

// String

func copyFromCType(_ handle: _baseRef) -> String {
    if let convertedString = String(data: Data(bytes: std_string_data_get(handle),
                                               count: Int(std_string_size_get(handle))),
                                    encoding: .utf8)
    {
        return convertedString
    }

    fatalError("Failed to decode character buffer as UTF-8 string")
}

func moveFromCType(_ handle: _baseRef) -> String {
    defer {
        std_string_release_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: String) -> RefHolder {
    RefHolder(std_string_create_handle(swiftType))
}

func moveToCType(_ swiftType: String) -> RefHolder {
    RefHolder(ref: copyToCType(swiftType).ref, release: std_string_release_handle)
}

func copyFromCType(_ handle: _baseRef) -> String? {
    guard handle != 0 else {
        return nil
    }
    let unwrappedHandle = std_string_unwrap_optional_handle(handle)
    return copyFromCType(unwrappedHandle) as String
}

func moveFromCType(_ handle: _baseRef) -> String? {
    defer {
        std_string_release_optional_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: String?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(std_string_create_optional_handle(swiftType))
}

func moveToCType(_ swiftType: String?) -> RefHolder {
    RefHolder(ref: copyToCType(swiftType).ref, release: std_string_release_optional_handle)
}

// Data

func copyFromCType(_ handle: _baseRef) -> Data {
    guard let byteArrayData = byteArray_data_get(handle) else {
        return Data()
    }
    return Data(bytes: byteArrayData, count: Int(byteArray_size_get(handle)))
}

func moveFromCType(_ handle: _baseRef) -> Data {
    defer {
        byteArray_release_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: Data) -> RefHolder {
    let handle = byteArray_create_handle()
    swiftType.withUnsafeBytes { (ptr: UnsafeRawBufferPointer) in
        let uint8ptr = ptr.baseAddress?.assumingMemoryBound(to: UInt8.self)
        byteArray_assign(handle, uint8ptr, ptr.count)
    }
    return RefHolder(handle)
}

func moveToCType(_ swiftType: Data) -> RefHolder {
    RefHolder(ref: copyToCType(swiftType).ref, release: byteArray_release_handle)
}

func copyFromCType(_ handle: _baseRef) -> Data? {
    guard handle != 0 else {
        return nil
    }
    let unwrappedHandle = byteArray_unwrap_optional_handle(handle)
    return copyFromCType(unwrappedHandle) as Data
}

func moveFromCType(_ handle: _baseRef) -> Data? {
    defer {
        byteArray_release_optional_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: Data?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    let handle = byteArray_create_optional_handle()
    swiftType.withUnsafeBytes { (ptr: UnsafeRawBufferPointer) in
        let uint8ptr = ptr.baseAddress?.assumingMemoryBound(to: UInt8.self)
        byteArray_assign_optional(handle, uint8ptr, swiftType.count)
    }
    return RefHolder(handle)
}

func moveToCType(_ swiftType: Data?) -> RefHolder {
    RefHolder(ref: copyToCType(swiftType).ref, release: byteArray_release_optional_handle)
}

// Date

func copyFromCType(_ seconds_since_epoch: Double) -> Date {
    Date(timeIntervalSince1970: seconds_since_epoch)
}

func moveFromCType(_ seconds_since_epoch: Double) -> Date {
    copyFromCType(seconds_since_epoch)
}

func copyToCType(_ swiftType: Date) -> PrimitiveHolder<Double> {
    PrimitiveHolder(swiftType.timeIntervalSince1970)
}

func moveToCType(_ swiftType: Date) -> PrimitiveHolder<Double> {
    copyToCType(swiftType)
}

func copyFromCType(_ handle: _baseRef) -> Date? {
    guard handle != 0 else {
        return nil
    }
    let unwrappedHandle = double_value_get(handle)
    return copyFromCType(unwrappedHandle) as Date
}

func moveFromCType(_ handle: _baseRef) -> Date? {
    defer {
        double_release_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: Date?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    return RefHolder(double_create_handle(copyToCType(swiftType).ref))
}

func moveToCType(_ swiftType: Date?) -> RefHolder {
    RefHolder(ref: copyToCType(swiftType).ref, release: double_release_handle)
}

// Locale

func copyFromCType(_ handle: _baseRef) -> Locale {
    let languageTag = moveFromCType(locale_get_language_tag(handle)) as String?
    if let languageTagUnwrapped = languageTag {
        // BCP 47 language tag takes precedence if present.
        return Locale(identifier: languageTagUnwrapped)
    }

    var components: [String: String] = [:]

    let languageCode = moveFromCType(locale_get_language_code(handle)) as String?
    if let languageCodeUnwrapped = languageCode {
        components["kCFLocaleLanguageCodeKey"] = languageCodeUnwrapped
    }
    let countryCode = moveFromCType(locale_get_country_code(handle)) as String?
    if let countryCodeUnwrapped = countryCode {
        components["kCFLocaleCountryCodeKey"] = countryCodeUnwrapped
    }
    let scriptCode = moveFromCType(locale_get_script_code(handle)) as String?
    if let scriptCodeUnwrapped = scriptCode {
        components["kCFLocaleScriptCodeKey"] = scriptCodeUnwrapped
    }

    return Locale(identifier: Locale.identifier(fromComponents: components))
}

func moveFromCType(_ handle: _baseRef) -> Locale {
    defer {
        locale_release_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: Locale) -> RefHolder {
    let languageCodeHandle = moveToCType(swiftType.languageCode)
    let countryCodeHandle = moveToCType(swiftType.regionCode)
    let scriptCodeHandle = moveToCType(swiftType.scriptCode)
    let languageTagHandle = moveToCType(swiftType.identifier)
    return RefHolder(locale_create_handle(languageCodeHandle.ref, countryCodeHandle.ref,
                                          scriptCodeHandle.ref, languageTagHandle.ref))
}

func moveToCType(_ swiftType: Locale) -> RefHolder {
    RefHolder(ref: copyToCType(swiftType).ref, release: locale_release_handle)
}

func copyFromCType(_ handle: _baseRef) -> Locale? {
    guard handle != 0 else {
        return nil
    }
    let unwrappedHandle = locale_unwrap_optional_handle(handle)
    return copyFromCType(unwrappedHandle) as Locale
}

func moveFromCType(_ handle: _baseRef) -> Locale? {
    defer {
        locale_release_optional_handle(handle)
    }
    return copyFromCType(handle)
}

func copyToCType(_ swiftType: Locale?) -> RefHolder {
    guard let swiftType else {
        return RefHolder(0)
    }
    let handle = copyToCType(swiftType).ref
    defer {
        locale_release_handle(handle)
    }
    return RefHolder(locale_create_optional_handle(handle))
}

func moveToCType(_ swiftType: Locale?) -> RefHolder {
    RefHolder(ref: copyToCType(swiftType).ref, release: locale_release_optional_handle)
}

// Primitives

// catch primitive types
func copyFromCType<T>(_ primitive: T) -> T {
    primitive
}

// catch primitive types
func moveFromCType<T>(_ primitive: T) -> T {
    primitive
}

// catch primitive types
func copyToCType<T>(_ primitive: T) -> PrimitiveHolder<T> {
    PrimitiveHolder(primitive)
}

// catch primitive types
func moveToCType<T>(_ primitive: T) -> PrimitiveHolder<T> {
    PrimitiveHolder(primitive)
}
