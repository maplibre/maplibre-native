#pragma once

#include <mln/platform/gl_functions.hpp>

#include <type_traits>

namespace mln {
namespace gl {

template <typename T>
class Enum {
public:
    using InType =
        std::conditional_t<std::is_same_v<std::underlying_type_t<T>, bool>, platform::GLboolean, platform::GLint>;
    using OutType =
        std::conditional_t<std::is_same_v<std::underlying_type_t<T>, bool>, platform::GLboolean, platform::GLenum>;

    static T from(InType);
    static OutType to(T);

    template <typename U>
    static OutType sizedFor(T, U type);
};

} // namespace gl
} // namespace mln
