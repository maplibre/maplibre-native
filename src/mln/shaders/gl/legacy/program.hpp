#pragma once

#include <mln/shaders/gl/legacy/program_base.hpp>
#include <mln/gfx/attribute.hpp>
#include <mln/gfx/shader.hpp>
#include <mln/gfx/uniform.hpp>
#include <mln/gfx/draw_mode.hpp>
#include <mln/shaders/segment.hpp>
#include <mln/shaders/attributes.hpp>
#include <mln/shaders/program_parameters.hpp>
#include <mln/shaders/shader_manifest.hpp>
#include <mln/style/paint_property.hpp>
#include <mln/renderer/paint_property_binder.hpp>
#include <mln/util/io.hpp>

#include <unordered_map>

namespace mln {

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
    bool draw(gfx::Context& context,
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
            return false;
        }

        auto drawScopeIt = segment.drawScopes.find(layerID);
        if (drawScopeIt == segment.drawScopes.end()) {
            drawScopeIt = segment.drawScopes.emplace(layerID, context.createDrawScope()).first;
        }

        return programBase->draw(context,
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
    bool draw(gfx::Context& context,
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
            return false;
        }

        for (auto& segment : segments) {
            auto drawScopeIt = segment.drawScopes.find(layerID);

            if (drawScopeIt == segment.drawScopes.end()) {
                drawScopeIt = segment.drawScopes.emplace(layerID, context.createDrawScope()).first;
            }

            if (!programBase->draw(context,
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
                                   segment.indexLength)) {
                return false;
            }
        }
        return true;
    }
};

class LayerTypePrograms {
public:
    virtual ~LayerTypePrograms() = default;
};

} // namespace mln
