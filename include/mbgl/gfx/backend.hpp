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
        Metal,    ///< The Metal API backend
        TYPE_MAX, ///< Not a valid backend type, used to determine the number
                  ///< of available backends (ie for array allocation).
    };

#if MLN_RENDER_BACKEND_METAL
    static constexpr Type DefaultType = Type::Metal;
#else // assume MLN_RENDER_BACKEND_OPENGL
    static constexpr Type DefaultType = Type::OpenGL;
#endif

    static void SetType(const Type value) {
        if (Value(value) != value) {
            abort(); // SetType must be called prior to any GetType calls.
        }
    }

    static Type GetType() { return Value(DefaultType); }

    static bool getEnableGPUExpressionEval() { return enableGPUExpressionEval; }
    static void setEnableGPUExpressionEval(bool value) { enableGPUExpressionEval = value; }

    template <typename T, typename... Args>
    static std::unique_ptr<T> Create(Args... args) {
#if MLN_RENDER_BACKEND_METAL
        return Create<Type::Metal, T, Args...>(std::forward<Args>(args)...);
#else // assume MLN_RENDER_BACKEND_OPENGL
        return Create<Type::OpenGL, T, Args...>(std::forward<Args>(args)...);
#endif
    }

private:
    template <Type, typename T, typename... Args>
    static std::unique_ptr<T> Create(Args...);

    static Type Value(Type value) {
        static const Type type = value;
        return type;
    }

    static bool enableGPUExpressionEval;
};

} // namespace gfx
} // namespace mbgl
