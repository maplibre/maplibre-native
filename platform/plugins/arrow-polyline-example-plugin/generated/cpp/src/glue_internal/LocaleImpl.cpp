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

#include "glue_internal/Locale.h"

namespace glue_internal {

Locale::Locale() noexcept = default;

Locale::Locale(std::optional<std::string> language_tag) noexcept
    : language_tag(std::move(language_tag)) {}

Locale::Locale(std::optional<std::string> language_code, std::optional<std::string> country_code) noexcept
    : language_code(std::move(language_code)),
      country_code(std::move(country_code)) {}

Locale::Locale(std::optional<std::string> language_code,
               std::optional<std::string> country_code,
               std::optional<std::string> script_code) noexcept
    : language_code(std::move(language_code)),
      country_code(std::move(country_code)),
      script_code(std::move(script_code)) {}

Locale::Locale(std::optional<std::string> language_code,
               std::optional<std::string> country_code,
               std::optional<std::string> script_code,
               std::optional<std::string> language_tag) noexcept
    : language_code(std::move(language_code)),
      country_code(std::move(country_code)),
      script_code(std::move(script_code)),
      language_tag(std::move(language_tag)) {}

Locale::Locale(std::string language_tag) noexcept
    : language_tag(std::optional<std::string>(std::move(language_tag))) {}

Locale::Locale(std::string language_code, std::string country_code) noexcept
    : language_code(std::optional<std::string>(std::move(language_code))),
      country_code(std::optional<std::string>(std::move(country_code))) {}

Locale::Locale(std::string language_code, std::string country_code, std::string script_code) noexcept
    : language_code(std::optional<std::string>(std::move(language_code))),
      country_code(std::optional<std::string>(std::move(country_code))),
      script_code(std::optional<std::string>(std::move(script_code))) {}

Locale::Locale(std::string language_code,
               std::string country_code,
               std::string script_code,
               std::string language_tag) noexcept
    : language_code(std::optional<std::string>(std::move(language_code))),
      country_code(std::optional<std::string>(std::move(country_code))),
      script_code(std::optional<std::string>(std::move(script_code))),
      language_tag(std::optional<std::string>(std::move(language_tag))) {}

bool Locale::operator==(const Locale& rhs) const {
    return (language_code == rhs.language_code) && (country_code == rhs.country_code) &&
           (script_code == rhs.script_code) && (language_tag == rhs.language_tag);
}

bool Locale::operator!=(const Locale& rhs) const {
    return !(*this == rhs);
}

std::size_t hash<Locale>::operator()(const Locale& t) const noexcept {
    size_t hash_value = 43;
    hash_value = (hash_value ^ hash<std::optional<std::string>>()(t.language_code)) << 1;
    hash_value = (hash_value ^ hash<std::optional<std::string>>()(t.country_code)) << 1;
    hash_value = (hash_value ^ hash<std::optional<std::string>>()(t.script_code)) << 1;
    hash_value = (hash_value ^ hash<std::optional<std::string>>()(t.language_tag)) << 1;
    return hash_value;
}

} // namespace glue_internal
