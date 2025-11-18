#pragma once

#include <algorithm>
#include <atomic>
#include <cassert>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <thread>
#include <type_traits>

namespace mapbox {
namespace base {

/// @cond internal
namespace internal {

class WeakPtrSharedData {
public:
    WeakPtrSharedData() = default;

    void sharedLock() {
        std::size_t noLocks = sharedLocks_;
        std::size_t incremented;
        do {
            if (noLocks == kInvalidValue) return;
            incremented = noLocks + 1;
            // `compare_exchange_weak()` is invoked in a cycle to handle the case,
            // if another thread has just modified `sharedLocks_` (so that it is not
            // equal to `noLocks` any more). We early return, if `sharedLocks_` became
            // equal to `kInvalidValue`.
        } while (!sharedLocks_.compare_exchange_weak(noLocks, incremented));
    }

    void sharedUnlock() {
        std::size_t noLocks = sharedLocks_;
        std::size_t decremented;
        do {
            if (noLocks == kInvalidValue) return;
            assert(noLocks > 0u);
            decremented = noLocks - 1;
        } while (!sharedLocks_.compare_exchange_weak(noLocks, decremented));
    }

    void invalidate() {
        assert(valid());
        std::size_t noLocks = 0u;
        while (!sharedLocks_.compare_exchange_weak(noLocks, kInvalidValue)) {
            assert(noLocks != kInvalidValue);
            noLocks = 0u;
        }
        assert(!valid());
    }

    bool valid() const { return sharedLocks_ != kInvalidValue; }

private:
    static constexpr size_t kInvalidValue = std::numeric_limits<std::size_t>::max();
    std::atomic_size_t sharedLocks_{0u};
};

using StrongRef = std::shared_ptr<WeakPtrSharedData>;
using WeakRef = std::weak_ptr<WeakPtrSharedData>;

template <typename T>
class WeakPtrBase;

} // namespace internal
/// @endcond

/**
 * @brief Scope guard for the WeakPtr-wrapped object
 *
 * The WeakPtr-wrapped object is guaranteed not to be deleted while
 * a WeakPtrGuard instance is present in the scope.
 */
class WeakPtrGuard {
public:
    /**
     * @brief Default move constructor
     */
    WeakPtrGuard(WeakPtrGuard&&) noexcept = default;
    ~WeakPtrGuard() {
        if (strong_) {
            strong_->sharedUnlock();
        }
    }

private:
    explicit WeakPtrGuard(internal::StrongRef strong) : strong_(std::move(strong)) {
        assert(!strong_ || strong_->valid());
    }
    internal::StrongRef strong_;

    template <typename T>
    friend class internal::WeakPtrBase;
};

namespace internal {

/**
 * @brief Base type for \c WeakPtr class.
 *
 * Contains the generic API for weak pointer classes.
 *
 * This class helps to create a \c WeakPtr template specialization for a
 * certain type, enabling the type-specific semantics.
 *
 * \sa WeakPtr
 *
 * @tparam T the managed object type
 */
template <typename T>
class WeakPtrBase {
public:
    /**
     * Gets a lock that protects the managed object, giving
     * the guarantee that it will not be deleted (if not
     * deleted yet).
     *
     * Note that it won't make the managed object thread-safe, but
     * rather make sure it exists (or not) when the lock
     * is being held.
     *
     * Note: there *MUST* be only one instance of the
     * guard referring to the same \a WeakPtrFactory
     * available in the scope at a time.
     */
    WeakPtrGuard lock() const {
        if (StrongRef strong = weak_.lock()) {
            strong->sharedLock();
            if (strong->valid()) {
                return WeakPtrGuard(std::move(strong));
            }
            strong->sharedUnlock();
        }
        return WeakPtrGuard(nullptr);
    }

    /**
     * @brief Quick nonblocking check that the managed object still exists.
     *
     * Checks if the weak pointer is still pointing to a valid
     * object. Note that if the WeakPtrFactory lives in a different
     * thread, a `false` result cannot be guaranteed to be
     * correct since there is an implicit race condition,
     * but a `true` result is always correct.
     *
     * @return given the thread restrictions, true if expired,
     * false otherwise.
     */
    bool expired() const {
        if (StrongRef strong = weak_.lock()) {
            return !strong->valid();
        }
        return true;
    }

    /**
     * @brief Quick nonblocking check that the managed object still exists.
     *
     * \sa expired()
     *
     * @return given the thread restrictions, true if the object exists,
     * false otherwise.
     */
    explicit operator bool() const { return !expired(); }

    /**
     * Get a raw pointer to the managed object.
     *
     * In multi-threaded environment, the caller *MUST* call
     * lock() and keep locker, before using it.
     *
     * Usage should be as brief as possible, because it might
     * potentially block the thread where the managed object lives.
     *
     * @return pointer to the object, nullptr if expired.
     */
    T* get() const {
        if (StrongRef strong = weak_.lock()) {
            if (strong->valid()) {
                return ptr_;
            }
        }
        return nullptr;
    }

protected:
    /// @cond internal
    WeakPtrBase() = default;
    WeakPtrBase(WeakPtrBase&&) noexcept = default;
    WeakPtrBase(const WeakPtrBase&) noexcept = default;
    template <typename U> // NOLINTNEXTLINE
    WeakPtrBase(WeakPtrBase<U>&& other) noexcept : weak_(std::move(other.weak_)), ptr_(static_cast<T*>(other.ptr_)) {}
    explicit WeakPtrBase(WeakRef weak, T* ptr) : weak_(std::move(weak)), ptr_(ptr) { assert(ptr_); }
    WeakPtrBase& operator=(WeakPtrBase&& other) noexcept = default;
    WeakPtrBase& operator=(const WeakPtrBase& other) = default;

    ~WeakPtrBase() = default;

private:
    WeakRef weak_;
    T* ptr_{};
    template <typename U>
    friend class WeakPtrBase;
    /// @endcond
};

} // namespace internal

/**
 * @brief Default implementation of a weak pointer to an object.
 *
 * Weak pointers are safe to access even if the
 * pointer outlives the object, which this class wraps.
 *
 * This class will manage only object lifetime
 * but will not deal with thread-safeness of the
 * objects it is wrapping.
 */
template <typename T>
class WeakPtr final : public internal::WeakPtrBase<T> {
public:
    /**
     * @brief Default constructor.
     *
     * Constructs empty \c WeakPtr.
     */
    WeakPtr() = default;

    /**
     * @brief Converting move constructor
     *
     * \a other becomes empty after the call.
     *
     * @tparam U a type, which \c T is convertible to
     * @param other \c WeakPtr<U> instance
     */
    template <typename U> // NOLINTNEXTLINE
    WeakPtr(WeakPtr<U>&& other) noexcept : internal::WeakPtrBase<T>(std::move(other)) {}

    /**
     * @brief Default move constructor.
     */
    WeakPtr(WeakPtr&&) noexcept = default;

    /**
     * @brief Default copy constructor.
     */
    WeakPtr(const WeakPtr&) noexcept = default;

    /**
     * @brief Replaces the managed object with the one managed by \a other.
     *
     *  \a other becomes empty after the call.
     *
     * @param other
     * @return WeakPtr&  \c *this
     */
    WeakPtr& operator=(WeakPtr&& other) noexcept = default;

    /**
     * @brief Replaces the managed object with the one managed by \a other.
     *
     * @param other
     * @return WeakPtr&  \c *this
     */
    WeakPtr& operator=(const WeakPtr& other) = default;

    /**
     * @brief Dereferences the stored pointer.
     *
     * Must not be called on empty \c WeakPtr.
     *
     * @return T*  the stored pointer.
     */
    T* operator->() const {
        T* ptr = this->get();
        assert(ptr);
        return ptr;
    }

private:
    explicit WeakPtr(internal::WeakRef weak, T* object) : internal::WeakPtrBase<T>(std::move(weak), object) {}

    template <typename U>
    friend class WeakPtrFactory;
};

/**
 * @brief T wrapper that can create weak pointers.
 *
 * WARNING: the WeakPtrFactory should all be at the bottom of
 * the list of member of the class, making it the first to
 * be destroyed and the last to be initialized.
 */
template <typename T>
class WeakPtrFactory final {
public:
    WeakPtrFactory(const WeakPtrFactory&) = delete;
    WeakPtrFactory& operator=(const WeakPtrFactory&) = delete;

    /**
     * @brief Construct a new \c WeakPtrFactory object.
     *
     * @param obj an \c T instance to wrap.
     */
    explicit WeakPtrFactory(T* obj) : strong_(std::make_shared<internal::WeakPtrSharedData>()), obj_(obj) {}

    /**
     * Destroys the factory, invalidating all the
     * weak pointers to this object, i.e. makes them empty.
     */
    ~WeakPtrFactory() { invalidateWeakPtrs(); }

    /**
     * Make a weak pointer for this WeakPtrFactory. Weak pointer
     * can be used for safely accessing the T object and not worry
     * about lifetime.
     *
     * @return a weak pointer.
     */
    WeakPtr<T> makeWeakPtr() { return WeakPtr<T>{strong_, obj_}; }

    /**
     * @brief Makes a weak wrapper for calling a method on the wrapped
     * \c T instance.
     *
     * While the wrapped \c T instance exists, calling the returned wrapper is
     * equivalent to invoking \a method on the instance. Note that the instance deletion
     * is blocked during the wrapper call.
     *
     * If the wrapped \c T instance does not exist, calling the returned wrapper
     * is ignored.
     *
     * The example below illustrates creating an \c std::function instance from the
     * returned wrapper.
     *
     * \code
     *  class T {
     *      void foo(int);
     *      std::function<void(int)> makeWeakFoo() {
     *	        return weakFactory.makeWeakMethod(&T::foo);
     *      }
     *      mapbox::base::WeakPtrFactory<T> weakFactory{this};
     *  };
     * \endcode
     *
     * @param method Pointer to an \c T class method.
     * @return auto Callable object
     */
    template <typename Method>
    auto makeWeakMethod(Method method) {
        return [weakPtr = makeWeakPtr(), method](auto&&... params) mutable {
            WeakPtrGuard guard = weakPtr.lock();
            if (T* obj = weakPtr.get()) {
                (obj->*method)(std::forward<decltype(params)>(params)...);
            }
        };
    }

    /**
     * @brief Invalidates all existing weak pointers.
     *
     * This method is particularly useful, when \c T  class has a user-defined destructor,
     * that makes the wrapped instance invalid even before entering the \c WeakPtrFactory
     * destructor.
     * In this case, clients *MUST* explicitly call \c invalidateWeakPtrs() before freeing any resources.
     *
     * Note: After \c invalidateWeakPtrs() is called, \c makeWeakPtr() returns empty weak pointers.
     */
    void invalidateWeakPtrs() {
        if (strong_) strong_->invalidate();
        strong_.reset();
    }

private:
    internal::StrongRef strong_;
    T* obj_;
};

} // namespace base
} // namespace mapbox
