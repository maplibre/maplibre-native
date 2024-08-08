#pragma once

#include <mbgl/gfx/backend.hpp>
#include <mbgl/shaders/shader_source.hpp>

#include <string_view>

namespace mbgl {
namespace gfx {

class ContextObserver {
public:
    virtual void onPreCompileShader(shaders::BuiltIn, gfx::Backend::Type){};
    virtual void onPostCompileShader(shaders::BuiltIn, gfx::Backend::Type){};
    virtual void onShaderCompileFailed(shaders::BuiltIn, gfx::Backend::Type){};

    virtual void onPreUploadTexture(gfx::Backend::Type){};
    virtual void onPostUploadTexture(gfx::Backend::Type){};

    virtual void onPreUploadUBO(gfx::Backend::Type){};
    virtual void onPostUploadUBO(gfx::Backend::Type){};

    virtual void onPreCreateRenderPipelineState(gfx::Backend::Type){};
    virtual void onPostCreateRenderPipelineState(gfx::Backend::Type){};
};

} // namespace gfx
} // namespace mbgl
