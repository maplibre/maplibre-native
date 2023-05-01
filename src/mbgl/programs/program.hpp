#pragma once

#include <mbgl/gfx/attribute.hpp>
#include <mbgl/gfx/shader.hpp>
#include <mbgl/gfx/uniform.hpp>
#include <mbgl/gfx/draw_mode.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/programs/attributes.hpp>
#include <mbgl/programs/program_parameters.hpp>
#include <mbgl/style/paint_property.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/util/io.hpp>

#include <unordered_map>

#include <mbgl/shaders/shader_manifest.hpp>
#ifdef MBGL_RENDER_BACKEND_OPENGL
#include <mbgl/gl/program.hpp>
#endif

namespace mbgl {

namespace gfx {
class RenderPass;
} // namespace gfx

template <class Name,
          shaders::BuiltIn ShaderSource,
          gfx::PrimitiveType Primitive,
          class LayoutAttributeList,
          class LayoutUniformList,
          class Textures,
          class PaintProps>
class Program : public gfx::Shader {
public:
    using LayoutVertex = gfx::Vertex<LayoutAttributeList>;

    using PaintProperties = PaintProps;
    using Binders = PaintPropertyBinders<typename PaintProperties::DataDrivenProperties>;

    using PaintAttributeList = typename Binders::AttributeList;
    using AttributeList = TypeListConcat<LayoutAttributeList, PaintAttributeList>;
    using AttributeBindings = gfx::AttributeBindings<AttributeList>;

    using PaintUniformList = typename Binders::UniformList;
    using UniformList = TypeListConcat<LayoutUniformList, PaintUniformList>;
    using LayoutUniformValues = gfx::UniformValues<LayoutUniformList>;
    using UniformValues = gfx::UniformValues<UniformList>;

    using TextureList = Textures;
    using TextureBindings = gfx::TextureBindings<TextureList>;

    std::unique_ptr<gfx::Program<Name>> program;

    Program(const ProgramParameters& programParameters) {
        switch (gfx::Backend::GetType()) {
#ifdef MBGL_RENDER_BACKEND_OPENGL
            case gfx::Backend::Type::OpenGL: {
                program = std::make_unique<gl::Program<Name>>(programParameters.withDefaultSource(
                    {gfx::Backend::Type::OpenGL,
                     shaders::ShaderSource<ShaderSource, gfx::Backend::Type::OpenGL>::vertex,
                     shaders::ShaderSource<ShaderSource, gfx::Backend::Type::OpenGL>::fragment}));
                break;
            }
#endif
            default: {
                throw std::runtime_error("Unsupported rendering backend!");
            }
        }
    }

    static UniformValues computeAllUniformValues(const LayoutUniformValues& layoutUniformValues,
                                                 const Binders& paintPropertyBinders,
                                                 const typename PaintProperties::PossiblyEvaluated& currentProperties,
                                                 float currentZoom) {
        return layoutUniformValues.concat(paintPropertyBinders.uniformValues(currentZoom, currentProperties));
    }

    static AttributeBindings computeAllAttributeBindings(
        const gfx::VertexBuffer<LayoutVertex>& layoutVertexBuffer,
        const Binders& paintPropertyBinders,
        const typename PaintProperties::PossiblyEvaluated& currentProperties) {
        return gfx::AttributeBindings<LayoutAttributeList>(layoutVertexBuffer)
            .concat(paintPropertyBinders.attributeBindings(currentProperties));
    }

    static uint32_t activeBindingCount(const AttributeBindings& allAttributeBindings) {
        return allAttributeBindings.activeCount();
    }

    template <class DrawMode>
    void draw(gfx::Context& context,
              gfx::RenderPass& renderPass,
              const DrawMode& drawMode,
              const gfx::DepthMode& depthMode,
              const gfx::StencilMode& stencilMode,
              const gfx::ColorMode& colorMode,
              const gfx::CullFaceMode& cullFaceMode,
              const gfx::IndexBuffer& indexBuffer,
              const Segment<AttributeList>& segment,
              const UniformValues& uniformValues,
              const AttributeBindings& allAttributeBindings,
              const TextureBindings& textureBindings,
              const std::string& layerID) {
        static_assert(Primitive == gfx::PrimitiveTypeOf<DrawMode>::value, "incompatible draw mode");

        if (!program) {
            return;
        }

        auto drawScopeIt = segment.drawScopes.find(layerID);
        if (drawScopeIt == segment.drawScopes.end()) {
            drawScopeIt = segment.drawScopes.emplace(layerID, context.createDrawScope()).first;
        }

        program->draw(context,
                      renderPass,
                      drawMode,
                      depthMode,
                      stencilMode,
                      colorMode,
                      cullFaceMode,
                      uniformValues,
                      drawScopeIt->second,
                      allAttributeBindings.offset(segment.vertexOffset),
                      textureBindings,
                      indexBuffer,
                      segment.indexOffset,
                      segment.indexLength);
    }

    template <class DrawMode>
    void draw(gfx::Context& context,
              gfx::RenderPass& renderPass,
              const DrawMode& drawMode,
              const gfx::DepthMode& depthMode,
              const gfx::StencilMode& stencilMode,
              const gfx::ColorMode& colorMode,
              const gfx::CullFaceMode& cullFaceMode,
              const gfx::IndexBuffer& indexBuffer,
              const SegmentVector<AttributeList>& segments,
              const UniformValues& uniformValues,
              const AttributeBindings& allAttributeBindings,
              const TextureBindings& textureBindings,
              const std::string& layerID) {
        static_assert(Primitive == gfx::PrimitiveTypeOf<DrawMode>::value, "incompatible draw mode");

        if (!program) {
            return;
        }

        for (auto& segment : segments) {
            auto drawScopeIt = segment.drawScopes.find(layerID);

            if (drawScopeIt == segment.drawScopes.end()) {
                drawScopeIt = segment.drawScopes.emplace(layerID, context.createDrawScope()).first;
            }

            program->draw(context,
                          renderPass,
                          drawMode,
                          depthMode,
                          stencilMode,
                          colorMode,
                          cullFaceMode,
                          uniformValues,
                          drawScopeIt->second,
                          allAttributeBindings.offset(segment.vertexOffset),
                          textureBindings,
                          indexBuffer,
                          segment.indexOffset,
                          segment.indexLength);
        }
    }
};

class LayerTypePrograms {
public:
    virtual ~LayerTypePrograms() = default;
};

} // namespace mbgl
