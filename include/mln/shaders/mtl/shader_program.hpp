#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <mln/shaders/shader_program_base.hpp>
#include <mln/mtl/mtl_fwd.hpp>
#include <mln/mtl/vertex_attribute.hpp>

#include <Foundation/NSSharedPtr.hpp>

#include <optional>
#include <string>
#include <unordered_map>

namespace mln {
namespace shaders {

struct AttributeInfo {
    constexpr AttributeInfo(std::size_t index_,
                            gfx::AttributeDataType dataType_,
                            std::size_t bufferIndex_,
                            std::size_t id_)
        : index(index_),
          dataType(dataType_),
          bufferIndex(bufferIndex_),
          id(id_) {}
    std::size_t index;
    gfx::AttributeDataType dataType;
    std::size_t bufferIndex;
    std::size_t id;
};

struct TextureInfo {
    constexpr TextureInfo(std::size_t index_, std::size_t id_)
        : index(index_),
          id(id_) {}
    std::size_t index;
    std::size_t id;
};

} // namespace shaders
namespace mtl {
class RenderableResource;
class RendererBackend;
class ShaderProgram;
using UniqueShaderProgram = std::unique_ptr<ShaderProgram>;

class ShaderProgram final : public gfx::ShaderProgramBase {
public:
    /// Functions of a program whose Metal library is compiling off the render thread
    /// (gfx::Context::setAsyncShaderCompilation). Filled in by the compile completion handler;
    /// the program adopts them on the render thread once `ready` is set.
    struct PendingFunctions {
        std::mutex mutex;
        MTLFunctionPtr vertex;
        MTLFunctionPtr fragment;
        std::atomic<bool> ready{false};
        std::atomic<bool> failed{false};
    };

    ShaderProgram(std::string name,
                  RendererBackend& backend,
                  MTLFunctionPtr vertexFunction,
                  MTLFunctionPtr fragmentFunction);
    /// A program still compiling: `getRenderPipelineState` returns null until the functions land.
    ShaderProgram(std::string name, RendererBackend& backend, std::shared_ptr<PendingFunctions> pending);
    ~ShaderProgram() noexcept override;

    static constexpr std::string_view Name{"GenericMTLShader"};
    const std::string_view typeName() const noexcept override { return Name; }

    /// Whether the compiled functions are available (always true for synchronously compiled
    /// programs). Drawables skip their draw while this is false; the renderer keeps requesting
    /// frames until it turns true.
    bool isReady() const;

    MTLRenderPipelineStatePtr getRenderPipelineState(const gfx::Renderable&,
                                                     const MTLVertexDescriptorPtr&,
                                                     const gfx::ColorMode& colorMode,
                                                     const std::optional<std::size_t> reuseHash) const;

    std::optional<size_t> getSamplerLocation(const size_t id) const override;

    const gfx::VertexAttributeArray& getVertexAttributes() const override { return vertexAttributes; }

    const gfx::VertexAttributeArray& getInstanceAttributes() const override { return instanceAttributes; }

    void initVertexAttribute(const shaders::AttributeInfo&);
    void initInstanceAttribute(const shaders::AttributeInfo&);
    void initTexture(const shaders::TextureInfo&);

protected:
    std::string shaderName;
    RendererBackend& backend;
    mutable MTLFunctionPtr vertexFunction;
    mutable MTLFunctionPtr fragmentFunction;
    mutable std::shared_ptr<PendingFunctions> pending;
    VertexAttributeArray vertexAttributes;
    VertexAttributeArray instanceAttributes;
    std::array<std::optional<size_t>, shaders::maxTextureCountPerShader> textureBindings;

    mutable mln::unordered_map<std::size_t, MTLRenderPipelineStatePtr> renderPipelineStateCache;
};

} // namespace mtl
} // namespace mln
