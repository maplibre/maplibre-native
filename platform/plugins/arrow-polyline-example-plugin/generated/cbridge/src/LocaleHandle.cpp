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

#include "cbridge/include/LocaleHandle.h"

#include "glue_internal//Locale.h"
#include "cbridge_internal/include/BaseHandleImpl.h"
#include <new>
#include <optional>
#include <string>

_baseRef
locale_create_handle(_baseRef language_code_handle,
                     _baseRef country_code_handle,
                     _baseRef script_code_handle,
                     _baseRef language_tag_handle)
{
    return reinterpret_cast<_baseRef>(
        new (::std::nothrow) ::glue_internal::Locale(
            Conversion<std::optional<std::string>>::toCpp(language_code_handle),
            Conversion<std::optional<std::string>>::toCpp(country_code_handle),
            Conversion<std::optional<std::string>>::toCpp(script_code_handle),
            Conversion<std::string>::toCpp(language_tag_handle)
        )
    );
}

void
locale_release_handle(_baseRef handle)
{
    delete get_pointer<::glue_internal::Locale>(handle);
}

_baseRef
locale_get_language_code(_baseRef handle)
{
    return Conversion<std::optional<std::string>>::toBaseRef(
        get_pointer<::glue_internal::Locale>(handle)->language_code
    );
}

_baseRef
locale_get_country_code(_baseRef handle)
{
    return Conversion<std::optional<std::string>>::toBaseRef(
        get_pointer<::glue_internal::Locale>(handle)->country_code
    );
}

_baseRef
locale_get_script_code(_baseRef handle)
{
    return Conversion<std::optional<std::string>>::toBaseRef(
        get_pointer<::glue_internal::Locale>(handle)->script_code
    );
}

_baseRef
locale_get_language_tag(_baseRef handle)
{
    return Conversion<std::optional<std::string>>::toBaseRef(
        get_pointer<::glue_internal::Locale>(handle)->language_tag
    );
}

_baseRef
locale_create_optional_handle(_baseRef locale_handle)
{
    return reinterpret_cast<_baseRef>(
        new (::std::nothrow) std::optional<::glue_internal::Locale>(
            *get_pointer<::glue_internal::Locale>(locale_handle)
        )
    );
}

void
locale_release_optional_handle(_baseRef handle)
{
    delete reinterpret_cast<std::optional<::glue_internal::Locale>*>(handle);
}

_baseRef
locale_unwrap_optional_handle(_baseRef handle)
{
    return reinterpret_cast<_baseRef>(
        &**reinterpret_cast<std::optional<::glue_internal::Locale>*>(handle)
    );
}
