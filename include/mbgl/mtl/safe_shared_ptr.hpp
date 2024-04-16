#pragma once

#include <type_traits>

#include <Foundation/NSSharedPtr.hpp>

#include <utility>

namespace mbgl {
namespace mtl {

/// Wrapper for `NS::SharedPtr` which prevents undefined behavior.
/// The base class relies on the caller to check before using `operator=`, `reset` etc. to avoid calling methods on null
/// pointers.
template <typename _Class>
class SafeSharedPtr : private NS::SharedPtr<_Class> {
    using Super = NS::SharedPtr<_Class>;

public:
    SafeSharedPtr() {}
    SafeSharedPtr(const Super& other) noexcept { operator=(other); }
    SafeSharedPtr(const SafeSharedPtr& other) noexcept { operator=(other); }
    SafeSharedPtr(Super&& other) noexcept { operator=(std::move(other)); }
    SafeSharedPtr(SafeSharedPtr&& other) noexcept { operator=(std::move(other)); }

    template <class _OtherClass>
    SafeSharedPtr(const NS::SharedPtr<_OtherClass>& other,
                  typename std::enable_if_t<std::is_convertible_v<_OtherClass*, _Class*>>* = nullptr) noexcept {
        operator=(other);
    }

    template <class _OtherClass>
    SafeSharedPtr(NS::SharedPtr<_OtherClass>&& other,
                  typename std::enable_if_t<std::is_convertible_v<_OtherClass*, _Class*>>* = nullptr) noexcept {
        operator=(std::move(other));
    }

    SafeSharedPtr& operator=(NS::SharedPtr<_Class>&& other) {
        if (other) {
            Super::operator=(std::move(other));
        } else {
            reset();
        }
        return *this;
    }

    SafeSharedPtr& operator=(const NS::SharedPtr<_Class>& other) {
        if (other) {
            Super::operator=(other);
        } else {
            reset();
        }
        return *this;
    }

    SafeSharedPtr& operator=(SafeSharedPtr<_Class>&& other) {
        if (other) {
            Super::operator=(std::move(other));
        } else {
            reset();
        }
        return *this;
    }

    SafeSharedPtr& operator=(const SafeSharedPtr<_Class>& other) {
        if (other) {
            Super::operator=(other);
        } else {
            reset();
        }
        return *this;
    }

    void reset() {
        if (Super::get()) {
            Super::reset();
        }
    }

    using Super::get;
    using Super::operator->;
    using Super::operator bool;
};

template <class _ClassLhs, class _ClassRhs>
inline bool operator==(const SafeSharedPtr<_ClassLhs>& lhs, const SafeSharedPtr<_ClassRhs>& rhs) {
    return lhs.get() == rhs.get();
}

template <class _ClassLhs, class _ClassRhs>
inline bool operator!=(const SafeSharedPtr<_ClassLhs>& lhs, const SafeSharedPtr<_ClassRhs>& rhs) {
    return lhs.get() != rhs.get();
}

} // namespace mtl
} // namespace mbgl
