#pragma once

#include <mln/gfx/types.hpp>
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

// Defined in enum.cpp. Declared here so that they cannot be instantiated before their definition.
#define MLN_DECLARE_GL_ENUM(T)        \
    template <>                       \
    T Enum<T>::from(Enum<T>::InType); \
    template <>                       \
    Enum<T>::OutType Enum<T>::to(T);

MLN_DECLARE_GL_ENUM(gfx::ColorBlendEquationType)
MLN_DECLARE_GL_ENUM(gfx::ColorBlendFactorType)
MLN_DECLARE_GL_ENUM(gfx::DepthFunctionType)
MLN_DECLARE_GL_ENUM(gfx::DepthMaskType)
MLN_DECLARE_GL_ENUM(gfx::StencilFunctionType)
MLN_DECLARE_GL_ENUM(gfx::StencilOpType)
MLN_DECLARE_GL_ENUM(gfx::CullFaceSideType)
MLN_DECLARE_GL_ENUM(gfx::CullFaceWindingType)
MLN_DECLARE_GL_ENUM(gfx::BufferUsageType)
MLN_DECLARE_GL_ENUM(gfx::TexturePixelType)
MLN_DECLARE_GL_ENUM(gfx::TextureChannelDataType)
MLN_DECLARE_GL_ENUM(gfx::RenderbufferPixelType)

template <>
platform::GLenum Enum<gfx::DrawModeType>::to(gfx::DrawModeType);

template <>
template <>
platform::GLenum Enum<gfx::TexturePixelType>::sizedFor<>(gfx::TexturePixelType, gfx::TextureChannelDataType);

#undef MLN_DECLARE_GL_ENUM

} // namespace gl
} // namespace mln
