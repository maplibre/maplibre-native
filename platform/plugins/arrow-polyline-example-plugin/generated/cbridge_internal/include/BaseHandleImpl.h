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

#pragma once

#include "cbridge/include/BaseHandle.h"

#include <chrono>
#include <cinttypes>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <memory>
#include <optional>

template <typename T>
inline static T* get_pointer(_baseRef handle) {
    return reinterpret_cast<T*>(handle);
}

template <typename T>
inline static ::std::shared_ptr<T>* checked_pointer_copy(::std::shared_ptr<T>&& pointer) {
    return !pointer ? nullptr : new ::std::shared_ptr<T>(::std::move(pointer));
}

template <typename T>
inline static ::std::shared_ptr<T>* checked_pointer_copy(const ::std::shared_ptr<T>& pointer) {
    return !pointer ? nullptr : new ::std::shared_ptr<T>(pointer);
}

template <class R, class... Args>
inline static ::std::function<R(Args...)>* checked_pointer_copy(const ::std::function<R(Args...)>& pointer) {
    return !pointer ? nullptr : new ::std::function<R(Args...)>(pointer);
}

template <typename T>
struct Conversion {
    static _baseRef toBaseRef(const T& t) { return reinterpret_cast<_baseRef>(new T(t)); }

    static _baseRef toBaseRef(T&& t) { return reinterpret_cast<_baseRef>(new T(::std::forward<T>(t))); }

    static _baseRef referenceBaseRef(const T& t) { return reinterpret_cast<_baseRef>(&t); }

    static T& toCpp(_baseRef ref) { return *reinterpret_cast<T*>(ref); }

    static T toCppReturn(_baseRef ref) {
        auto ptr = reinterpret_cast<T*>(ref);
        T value(*ptr);
        delete ptr;
        return value;
    }
};

template <class T>
struct Conversion<::std::shared_ptr<T>> {
    static _baseRef toBaseRef(::std::shared_ptr<T> ptr) {
        return !ptr ? 0 : reinterpret_cast<_baseRef>(new ::std::shared_ptr<T>(::std::move(ptr)));
    }

    static _baseRef referenceBaseRef(const ::std::shared_ptr<T>& t) { return reinterpret_cast<_baseRef>(&t); }

    static ::std::shared_ptr<T> toCpp(_baseRef ref) {
        if (ref == 0) {
            return {};
        }
        return *reinterpret_cast<::std::shared_ptr<T>*>(ref);
    }

    static ::std::shared_ptr<T> toCppReturn(_baseRef ref) {
        if (ref == 0) {
            return {};
        }
        auto ptr_ptr = reinterpret_cast<::std::shared_ptr<T>*>(ref);
        ::std::shared_ptr<T> ptr(::std::move(*ptr_ptr));
        delete ptr_ptr;
        return ptr;
    }
};

namespace {
template <class Rep, class Period>
double duration_to_seconds_double(const std::chrono::duration<Rep, Period>& d) {
    // Double can represent an integer up to 52 bits without a loss of precision. For big 64-bit
    // integers there is a loss, so those should be divided as integers, not as doubles.
    auto division_results = std::lldiv(d.count() * Period::num, Period::den);
    return static_cast<double>(division_results.quot) + static_cast<double>(division_results.rem) / Period::den;
}

template <class Rep, class Period>
std::chrono::duration<Rep, Period> seconds_double_to_duration(double seconds_double) {
    // Double can represent an integer up to 52 bits without a loss of precision. For big 64-bit
    // integers there is a loss, so those should be multiplied as integers, not as doubles.

    auto seconds_scaled = seconds_double / Period::num;
    auto integralPart = std::floor(seconds_scaled);
    auto decimalPart = seconds_scaled - integralPart;

    auto tick_count = static_cast<int64_t>(integralPart) * Period::den +
                      static_cast<int64_t>(std::round(decimalPart * Period::den));
    return std::chrono::duration<Rep, Period>(tick_count);
}
} // namespace

template <class T>
struct Conversion<std::optional<T>> {
    static _baseRef toBaseRef(std::optional<T> ptr) {
        return !ptr ? 0 : reinterpret_cast<_baseRef>(new std::optional<T>(::std::move(ptr)));
    }

    static _baseRef referenceBaseRef(const std::optional<T>& t) { return reinterpret_cast<_baseRef>(&t); }

    static std::optional<T> toCpp(_baseRef ref) {
        if (ref == 0) {
            return {};
        }
        return *reinterpret_cast<std::optional<T>*>(ref);
    }

    static std::optional<T> toCppReturn(_baseRef ref) {
        if (ref == 0) {
            return {};
        }
        auto ptr_ptr = reinterpret_cast<std::optional<T>*>(ref);
        std::optional<T> ptr(::std::move(*ptr_ptr));
        delete ptr_ptr;
        return ptr;
    }
};

template <class Clock, class Duration>
struct Conversion<std::chrono::time_point<Clock, Duration>> {
    static double toBaseRef(const std::chrono::time_point<Clock, Duration>& timestamp) {
        return duration_to_seconds_double(timestamp.time_since_epoch());
    }

    static double toBaseRef(std::chrono::time_point<Clock, Duration>&& timestamp) {
        return duration_to_seconds_double(timestamp.time_since_epoch());
    }

    static double referenceBaseRef(const std::chrono::time_point<Clock, Duration>& timestamp) {
        return duration_to_seconds_double(timestamp.time_since_epoch());
    }

    static std::chrono::time_point<Clock, Duration> toCpp(double seconds_value) {
        using namespace ::std::chrono;
        return time_point<Clock, Duration>(
            seconds_double_to_duration<typename Duration::rep, typename Duration::period>(seconds_value));
    }

    static std::chrono::time_point<Clock, Duration> toCppReturn(double seconds_value) {
        using namespace ::std::chrono;
        return time_point<Clock, Duration>(
            seconds_double_to_duration<typename Duration::rep, typename Duration::period>(seconds_value));
    }
};

template <class Clock, class Duration>
struct Conversion<std::optional<std::chrono::time_point<Clock, Duration>>> {
    static _baseRef toBaseRef(std::optional<std::chrono::time_point<Clock, Duration>> ptr) {
        if (!ptr) return 0;
        return reinterpret_cast<_baseRef>(
            new std::optional<double>(duration_to_seconds_double(ptr->time_since_epoch())));
    }

    static _baseRef referenceBaseRef(const std::optional<std::chrono::time_point<Clock, Duration>>& ptr) {
        return toBaseRef(ptr);
    }

    static std::optional<std::chrono::time_point<Clock, Duration>> toCpp(_baseRef ref) {
        if (ref == 0) return {};
        auto optional_double = *reinterpret_cast<std::optional<double>*>(ref);
        if (!optional_double) return {};
        return std::optional<std::chrono::time_point<Clock, Duration>>(std::chrono::time_point<Clock, Duration>(
            seconds_double_to_duration<typename Duration::rep, typename Duration::period>(*optional_double)));
    }

    static std::optional<std::chrono::time_point<Clock, Duration>> toCppReturn(_baseRef ref) {
        if (ref == 0) return {};
        auto optional_ptr = reinterpret_cast<std::optional<double>*>(ref);
        auto result =
            !*optional_ptr
                ? std::optional<std::chrono::time_point<Clock, Duration>>{}
                : std::optional<std::chrono::time_point<Clock, Duration>>(std::chrono::time_point<Clock, Duration>(
                      seconds_double_to_duration<typename Duration::rep, typename Duration::period>(**optional_ptr)));
        delete optional_ptr;
        return result;
    }
};

template <class Rep, class Period>
struct Conversion<std::chrono::duration<Rep, Period>> {
    static double toBaseRef(const std::chrono::duration<Rep, Period>& d) { return duration_to_seconds_double(d); }

    static double toBaseRef(const std::chrono::duration<Rep, Period>&& d) { return duration_to_seconds_double(d); }

    static double referenceBaseRef(const std::chrono::duration<Rep, Period>& d) {
        return duration_to_seconds_double(d);
    }

    static std::chrono::duration<Rep, Period> toCpp(double seconds_double) {
        return seconds_double_to_duration<Rep, Period>(seconds_double);
    }

    static std::chrono::duration<Rep, Period> toCppReturn(double seconds_double) {
        return seconds_double_to_duration<Rep, Period>(seconds_double);
    }
};

template <class Rep, class Period>
struct Conversion<std::optional<std::chrono::duration<Rep, Period>>> {
    static _baseRef toBaseRef(std::optional<std::chrono::duration<Rep, Period>> ptr) {
        if (!ptr) return 0;
        return reinterpret_cast<_baseRef>(new std::optional<double>(duration_to_seconds_double(*ptr)));
    }

    static _baseRef referenceBaseRef(const std::optional<std::chrono::duration<Rep, Period>>& ptr) {
        return toBaseRef(ptr);
    }

    static std::optional<std::chrono::duration<Rep, Period>> toCpp(_baseRef ref) {
        if (ref == 0) return {};
        auto optional_double = *reinterpret_cast<std::optional<double>*>(ref);
        if (!optional_double) return {};
        return std::optional<std::chrono::duration<Rep, Period>>(
            seconds_double_to_duration<Rep, Period>(*optional_double));
    }

    static std::optional<std::chrono::duration<Rep, Period>> toCppReturn(_baseRef ref) {
        if (ref == 0) return {};
        auto optional_ptr = reinterpret_cast<std::optional<double>*>(ref);
        auto result = !*optional_ptr ? std::optional<std::chrono::duration<Rep, Period>>{}
                                     : std::optional<std::chrono::duration<Rep, Period>>(
                                           seconds_double_to_duration<Rep, Period>(**optional_ptr));
        delete optional_ptr;
        return result;
    }
};
