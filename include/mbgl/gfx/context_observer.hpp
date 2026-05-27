#pragma once

#include <mbgl/gfx/backend.hpp>
#include <mbgl/shaders/shader_source.hpp>

#include <string>

namespace mbgl {
namespace gfx {

class ContextObserver {
public:
    virtual void onPreCompileShader(shaders::BuiltIn, gfx::Backend::Type, const std::string&) {}
    virtual void onPostCompileShader(shaders::BuiltIn, gfx::Backend::Type, const std::string&) {}
    virtual void onShaderCompileFailed(shaders::BuiltIn, gfx::Backend::Type, const std::string&) {}
};

} // namespace gfx
} // namespace mbgl
