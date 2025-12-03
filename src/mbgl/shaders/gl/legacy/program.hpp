#pragma once

#include <mbgl/shaders/gl/legacy/program_base.hpp>
#include <mbgl/gfx/attribute.hpp>
#include <mbgl/gfx/shader.hpp>
#include <mbgl/gfx/uniform.hpp>
#include <mbgl/gfx/draw_mode.hpp>
#include <mbgl/shaders/segment.hpp>
#include <mbgl/shaders/attributes.hpp>
#include <mbgl/shaders/program_parameters.hpp>
#include <mbgl/shaders/shader_manifest.hpp>
#include <mbgl/style/paint_property.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/util/io.hpp>

#include <unordered_map>

namespace mbgl {

namespace gfx {
class RenderPass;
} // namespace gfx

template <class Name,
          shaders::BuiltIn ShaderSource,
          gfx::PrimitiveType Primitive,
          class LayoutAttributeList,
          class LayoutUniformList,
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

    std::unique_ptr<gl::ProgramBase<Name>> programBase;

    // NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility) We don't want to list every derived class as a friend
    Program([[maybe_unused]] const ProgramParameters& programParameters) {
        switch (gfx::Backend::GetType()) {
#if MLN_RENDER_BACKEND_METAL
            case gfx::Backend::Type::Metal: {
                break;
            }
#elif MLN_RENDER_BACKEND_VULKAN
            case gfx::Backend::Type::Vulkan: {
                break;
            }
#elif MLN_RENDER_BACKEND_OPENGL
            case gfx::Backend::Type::OpenGL: {
                programBase = std::make_unique<gl::ProgramBase<Name>>(programParameters.withDefaultSource(
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
              const SegmentBase& segment,
              const UniformValues& uniformValues,
              const AttributeBindings& allAttributeBindings,
              const std::string& layerID) {
        static_assert(Primitive == gfx::PrimitiveTypeOf<DrawMode>::value, "incompatible draw mode");

        if (!programBase) {
            return;
        }

        auto drawScopeIt = segment.drawScopes.find(layerID);
        if (drawScopeIt == segment.drawScopes.end()) {
            drawScopeIt = segment.drawScopes.emplace(layerID, context.createDrawScope()).first;
        }

        programBase->draw(context,
                          renderPass,
                          drawMode,
                          depthMode,
                          stencilMode,
                          colorMode,
                          cullFaceMode,
                          uniformValues,
                          drawScopeIt->second,
                          allAttributeBindings.offset(segment.vertexOffset),
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
              const SegmentVector& segments,
              const UniformValues& uniformValues,
              const AttributeBindings& allAttributeBindings,
              const std::string& layerID) {
        static_assert(Primitive == gfx::PrimitiveTypeOf<DrawMode>::value, "incompatible draw mode");

        if (!programBase) {
            return;
        }

        for (auto& segment : segments) {
            auto drawScopeIt = segment.drawScopes.find(layerID);

            if (drawScopeIt == segment.drawScopes.end()) {
                drawScopeIt = segment.drawScopes.emplace(layerID, context.createDrawScope()).first;
            }

            programBase->draw(context,
                              renderPass,
                              drawMode,
                              depthMode,
                              stencilMode,
                              colorMode,
                              cullFaceMode,
                              uniformValues,
                              drawScopeIt->second,
                              allAttributeBindings.offset(segment.vertexOffset),
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
