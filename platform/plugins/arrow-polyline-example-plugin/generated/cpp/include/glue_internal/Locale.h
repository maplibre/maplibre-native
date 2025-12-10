// -------------------------------------------------------------------------------------------------
// Copyright (C) 2016-2020 HERE Europe B.V.
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

#pragma once

#include "ExportCommonGluecodiumCpp.h"
#include "Hash.h"
#include <optional>
#include <string>

namespace glue_internal {

/**
 * Represents an ISO locale, optionally with a corresponding BCP 47 language tag.
 */
struct _GLUECODIUM_CPP_EXPORT Locale final {
    Locale() noexcept;
    explicit Locale(std::optional<std::string> language_tag) noexcept;
    Locale(std::optional<std::string> language_code, std::optional<std::string> country_code) noexcept;
    Locale(std::optional<std::string> language_code,
           std::optional<std::string> country_code,
           std::optional<std::string> script_code) noexcept;
    Locale(std::optional<std::string> language_code,
           std::optional<std::string> country_code,
           std::optional<std::string> script_code,
           std::optional<std::string> language_tag) noexcept;
    explicit Locale(std::string language_tag) noexcept;
    Locale(std::string language_code, std::string country_code) noexcept;
    Locale(std::string language_code, std::string country_code, std::string script_code) noexcept;
    Locale(std::string language_code,
           std::string country_code,
           std::string script_code,
           std::string language_tag) noexcept;

    /// ISO 639-1 language code (2-letter)
    std::optional<std::string> language_code;
    /// ISO 3166-1 alpha-2 country code (2-letter)
    std::optional<std::string> country_code;
    /// ISO 15924 script code (4-letter)
    std::optional<std::string> script_code;
    /// BCP 47 language tag
    std::optional<std::string> language_tag;

    bool operator==(const Locale& rhs) const;
    bool operator!=(const Locale& rhs) const;
};

template <>
struct hash<Locale> {
    size_t operator()(const Locale& t) const noexcept;
};

} // namespace glue_internal
