#pragma once

#include <mln/gfx/backend.hpp>
#include <mln/shaders/shader_source.hpp>

#include <string>

namespace mln {
namespace gfx {

class ContextObserver {
public:
    virtual void onPreCompileShader(shaders::BuiltIn, gfx::Backend::Type, const std::string&) {}
    virtual void onPostCompileShader(shaders::BuiltIn, gfx::Backend::Type, const std::string&) {}
    virtual void onShaderCompileFailed(shaders::BuiltIn, gfx::Backend::Type, const std::string&) {}
    virtual void onRenderError(std::exception_ptr) {}
};

} // namespace gfx
} // namespace mln
