#include <mbgl/mtl/drawable.hpp>

#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/drawable_impl.hpp>
#include <mbgl/mtl/texture2d.hpp>
#include <mbgl/mtl/upload_pass.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace mtl {

Drawable::Drawable(std::string name_)
    : gfx::Drawable(std::move(name_)),
      impl(std::make_unique<Impl>()) {}

Drawable::~Drawable() {
    //impl->indexBuffer = {0, nullptr};
    //impl->attributeBuffers.clear();
}

void Drawable::draw(PaintParameters& parameters) const {
    if (isCustom) {
        return;
    }

    auto& context = static_cast<Context&>(parameters.context);

    if (shader) {
        const auto& shaderMTL = static_cast<const ShaderProgram&>(*shader);
        const auto& state = shaderMTL.getRenderPipelineState();
//        if (shaderGL.getGLProgramID() != context.program.getCurrentValue()) {
//            context.program = shaderGL.getGLProgramID();
//        }
    }
    /*
    if (!shader || context.program.getCurrentValue() == 0) {
        mbgl::Log::Warning(Event::General, "Missing shader for drawable " + util::toString(getID()) + "/" + getName());
        assert(false);
        return;
    }

    context.setDepthMode(getIs3D() ? parameters.depthModeFor3D()
                                   : parameters.depthModeForSublayer(getSubLayerIndex(), getDepthType()));

    // force disable depth test for debugging
    // context.setDepthMode({gfx::DepthFunctionType::Always, gfx::DepthMaskType::ReadOnly, {0,1}});

    // For 3D mode, stenciling is handled by the layer group
    if (!is3D) {
        context.setStencilMode(makeStencilMode(parameters));
    }

    context.setColorMode(getColorMode());
    context.setCullFaceMode(getCullFaceMode());

    bindUniformBuffers();
    bindTextures();

    for (const auto& seg : impl->segments) {
        const auto& glSeg = static_cast<DrawSegmentGL&>(*seg);
        const auto& mlSeg = glSeg.getSegment();
        if (mlSeg.indexLength > 0 && glSeg.getVertexArray().isValid()) {
            context.bindVertexArray = glSeg.getVertexArray().getID();
            context.draw(glSeg.getMode(), mlSeg.indexOffset, mlSeg.indexLength);
        }
    }

    context.bindVertexArray = value::BindVertexArray::Default;

    unbindTextures();
    unbindUniformBuffers();
     */
}

void Drawable::setIndexData(gfx::IndexVectorBasePtr indexes, std::vector<UniqueDrawSegment> segments) {
    //impl->indexes = std::move(indexes);
    //impl->segments = std::move(segments);
}

void Drawable::setVertices(std::vector<uint8_t>&& data, std::size_t count, gfx::AttributeDataType type) {
    //impl->vertexData = std::move(data);
    //impl->vertexCount = count;
    //impl->vertexType = type;
}

const gfx::VertexAttributeArray& Drawable::getVertexAttributes() const {
    return impl->vertexAttributes;
}

gfx::VertexAttributeArray& Drawable::mutableVertexAttributes() {
    return impl->vertexAttributes;
}

void Drawable::setVertexAttributes(const gfx::VertexAttributeArray& value) {
    //impl->vertexAttributes = static_cast<const VertexAttributeArrayGL&>(value);
}
void Drawable::setVertexAttributes(gfx::VertexAttributeArray&& value) {
    //impl->vertexAttributes = std::move(static_cast<VertexAttributeArrayGL&&>(value));
}

const gfx::UniformBufferArray& Drawable::getUniformBuffers() const {
    return impl->uniformBuffers;
}

gfx::UniformBufferArray& Drawable::mutableUniformBuffers() {
    return impl->uniformBuffers;
}
/*
void Drawable::setVertexAttrName(std::string value) {
    impl->vertexAttrName = std::move(value);
}

void Drawable::bindUniformBuffers() const {
    if (shader) {
        const auto& shaderGL = static_cast<const ShaderProgramGL&>(*shader);
        for (const auto& element : shaderGL.getUniformBlocks().getMap()) {
            const auto& uniformBuffer = getUniformBuffers().get(element.first);
            if (!uniformBuffer) {
                using namespace std::string_literals;
                Log::Error(Event::General,
                           "Drawable::bindUniformBuffers: UBO "s + element.first + " not found. skipping.");
                assert(false);
                continue;
            }
            element.second->bindBuffer(*uniformBuffer);
        }
    }
}

void Drawable::unbindUniformBuffers() const {
    if (shader) {
        const auto& shaderGL = static_cast<const ShaderProgramGL&>(*shader);
        for (const auto& element : shaderGL.getUniformBlocks().getMap()) {
            element.second->unbindBuffer();
        }
    }
}

void Drawable::upload(gfx::UploadPass& uploadPass) {
    if (!shader) {
        return;
    }

    const bool build = impl->vertexAttributes.isDirty() ||
                       std::any_of(impl->segments.begin(), impl->segments.end(), [](const auto& seg) {
                           return !static_cast<const DrawSegmentGL&>(*seg).getVertexArray().isValid();
                       });

    if (build) {
        auto& context = uploadPass.getContext();
        auto& glContext = static_cast<gl::Context&>(context);
        constexpr auto usage = gfx::BufferUsageType::StaticDraw;

        const auto indexBytes = impl->indexes->elements() * sizeof(gfx::IndexVectorBase::value_type);
        auto indexBufferResource = uploadPass.createIndexBufferResource(impl->indexes->data(), indexBytes, usage);
        auto indexBuffer = gfx::IndexBuffer{impl->indexes->elements(), std::move(indexBufferResource)};

        // Apply drawable values to shader defaults
        const auto& defaults = shader->getVertexAttributes();
        const auto& overrides = impl->vertexAttributes;

        const auto& indexAttribute = defaults.get(impl->vertexAttrName);
        const auto vertexAttributeIndex = static_cast<std::size_t>(indexAttribute ? indexAttribute->getIndex() : -1);

        std::vector<std::unique_ptr<gfx::VertexBufferResource>> vertexBuffers;
        auto bindings = uploadPass.buildAttributeBindings(impl->vertexCount,
                                                          impl->vertexType,
                                                          vertexAttributeIndex,
                                                          impl->vertexData,
                                                          defaults,
                                                          overrides,
                                                          usage,
                                                          vertexBuffers);

        impl->attributeBuffers = std::move(vertexBuffers);
        impl->indexBuffer = std::move(indexBuffer);

        // Create a VAO for each group of vertexes described by a segment
        for (const auto& seg : impl->segments) {
            auto& glSeg = static_cast<DrawSegmentGL&>(*seg);
            const auto& mlSeg = glSeg.getSegment();

            if (mlSeg.indexLength == 0) {
                continue;
            }

            for (auto& binding : bindings) {
                if (binding) {
                    binding->vertexOffset = static_cast<uint32_t>(mlSeg.vertexOffset);
                }
            }

            auto vertexArray = glContext.createVertexArray();

            vertexArray.bind(glContext, impl->indexBuffer, bindings);

            assert(vertexArray.isValid());

            glSeg.setVertexArray(std::move(vertexArray));
        };
    }

    const bool texturesNeedUpload = std::any_of(
        textures.begin(), textures.end(), [](const auto& pair) { return pair.second && pair.second->needsUpload(); });

    if (texturesNeedUpload) {
        uploadTextures();
    }
}

gfx::ColorMode Drawable::makeColorMode(PaintParameters& parameters) const {
    return enableColor ? parameters.colorModeForRenderPass() : gfx::ColorMode::disabled();
}

gfx::StencilMode Drawable::makeStencilMode(PaintParameters& parameters) const {
    if (enableStencil) {
        if (!is3D && tileID) {
            return parameters.stencilModeForClipping(tileID->toUnwrapped());
        }
        assert(false);
    }
    return gfx::StencilMode::disabled();
}

void Drawable::uploadTextures() const {
    for (const auto& pair : textures) {
        if (const auto& tex = pair.second) {
            std::static_pointer_cast<gl::Texture2D>(tex)->upload();
        }
    }
}

void Drawable::bindTextures() const {
    int32_t unit = 0;
    for (const auto& pair : textures) {
        if (const auto& tex = pair.second) {
            const auto& location = pair.first;
            std::static_pointer_cast<gl::Texture2D>(tex)->bind(location, unit++);
        }
    }
}

void Drawable::unbindTextures() const {
    for (const auto& pair : textures) {
        if (const auto& tex = pair.second) {
            std::static_pointer_cast<gl::Texture2D>(tex)->unbind();
        }
    }
}
 */
} // namespace mtl
} // namespace mbgl
