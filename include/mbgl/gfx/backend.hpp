#pragma once

#include <cstdint>
#include <cstdlib>
#include <memory>

namespace mbgl {
namespace gfx {

class Backend {
public:
    /// @brief The active graphics API/backend type.
    enum class Type : uint8_t {
        OpenGL,   ///< The OpenGL API backend
        TYPE_MAX, ///< Not a valid backend type, used to determine the number
                  ///< of available backends (ie for array allocation).
    };

    static constexpr Type DefaultType = Type::OpenGL;

    static void SetType(const Type value) {
        if (Value(value) != value) {
            abort(); // SetType must be called prior to any GetType calls.
        }
    }

    static Type GetType() { return Value(DefaultType); }

    template <typename T, typename... Args>
    static std::unique_ptr<T> Create(Args... args) {
        return Create<Type::OpenGL, T, Args...>(std::forward<Args>(args)...);
    }

private:
    template <Type, typename T, typename... Args>
    static std::unique_ptr<T> Create(Args...);

    static Type Value(Type value) {
        static const Type type = value;
        return type;
    }
};

} // namespace gfx
} // namespace mbgl
