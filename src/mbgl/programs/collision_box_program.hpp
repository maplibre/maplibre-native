#pragma once

#include <mbgl/programs/program.hpp>
#include <mbgl/programs/attributes.hpp>
#include <mbgl/programs/uniforms.hpp>
#include <mbgl/style/properties.hpp>
#include <mbgl/util/geometry.hpp>

#include <cmath>

namespace mbgl {

using CollisionBoxLayoutAttributes = TypeList<attributes::pos, attributes::anchor_pos, attributes::extrude>;

using CollisionBoxDynamicAttributes = TypeList<attributes::placed, attributes::shift>;

class CollisionBoxProgram final
    : public Program<CollisionBoxProgram,
                     shaders::BuiltIn::CollisionBoxProgram,
                     gfx::PrimitiveType::Line,
                     TypeListConcat<CollisionBoxLayoutAttributes, CollisionBoxDynamicAttributes>,
                     TypeList<uniforms::matrix, uniforms::extrude_scale, uniforms::camera_to_center_distance>,
                     TypeList<>,
                     style::Properties<>> {
public:
    static constexpr std::string_view Name{"CollisionBoxProgram"};
    const std::string_view typeName() const noexcept override { return Name; }

    using Program::Program;

    static gfx::Vertex<CollisionBoxLayoutAttributes> layoutVertex(Point<float> a, Point<float> anchor, Point<float> o) {
        return {{{static_cast<int16_t>(a.x), static_cast<int16_t>(a.y)}},
                {{static_cast<int16_t>(anchor.x), static_cast<int16_t>(anchor.y)}},
                {{static_cast<int16_t>(::round(o.x)), static_cast<int16_t>(::round(o.y))}}};
    }

    static gfx::Vertex<CollisionBoxDynamicAttributes> dynamicVertex(bool placed, bool notUsed, Point<float> shift) {
        return {{{static_cast<uint16_t>(placed), static_cast<uint16_t>(notUsed)}}, {{shift.x, shift.y}}};
    }

    template <class DrawMode>
    void draw(gfx::Context& context,
              gfx::RenderPass& renderPass,
              const DrawMode& drawMode,
              const gfx::DepthMode& depthMode,
              const gfx::StencilMode& stencilMode,
              const gfx::ColorMode& colorMode,
              const gfx::CullFaceMode& cullFaceMode,
              const LayoutUniformValues& layoutUniformValues,
              const gfx::VertexBuffer<gfx::Vertex<CollisionBoxLayoutAttributes>>& layoutVertexBuffer,
              const gfx::VertexBuffer<gfx::Vertex<CollisionBoxDynamicAttributes>>& dynamicVertexBuffer,
              const gfx::IndexBuffer& indexBuffer,
              const SegmentVector<AttributeList>& segments,
              const Binders& paintPropertyBinders,
              const typename PaintProperties::PossiblyEvaluated& currentProperties,
              const TextureBindings& textureBindings,
              float currentZoom,
              const std::string& layerID) {
        UniformValues uniformValues = layoutUniformValues.concat(
            paintPropertyBinders.uniformValues(currentZoom, currentProperties));

        AttributeBindings allAttributeBindings =
            gfx::AttributeBindings<CollisionBoxLayoutAttributes>(layoutVertexBuffer)
                .concat(gfx::AttributeBindings<CollisionBoxDynamicAttributes>(dynamicVertexBuffer))
                .concat(paintPropertyBinders.attributeBindings(currentProperties));

        assert(layoutVertexBuffer.elements == dynamicVertexBuffer.elements);

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

class CollisionCircleProgram final
    : public Program<CollisionCircleProgram,
                     shaders::BuiltIn::CollisionCircleProgram,
                     gfx::PrimitiveType::Triangle,
                     TypeListConcat<CollisionBoxLayoutAttributes, CollisionBoxDynamicAttributes>,
                     TypeList<uniforms::matrix,
                              uniforms::extrude_scale,
                              uniforms::overscale_factor,
                              uniforms::camera_to_center_distance>,
                     TypeList<>,
                     style::Properties<>> {
public:
    static constexpr std::string_view Name{"CollisionCircleProgram"};
    const std::string_view typeName() const noexcept override { return Name; }

    using Program::Program;

    static gfx::Vertex<CollisionBoxLayoutAttributes> vertex(Point<float> a, Point<float> anchor, Point<float> o) {
        return {{{static_cast<int16_t>(a.x), static_cast<int16_t>(a.y)}},
                {{static_cast<int16_t>(anchor.x), static_cast<int16_t>(anchor.y)}},
                {{static_cast<int16_t>(::round(o.x)), static_cast<int16_t>(::round(o.y))}}};
    }

    template <class DrawMode>
    void draw(gfx::Context& context,
              gfx::RenderPass& renderPass,
              const DrawMode& drawMode,
              const gfx::DepthMode& depthMode,
              const gfx::StencilMode& stencilMode,
              const gfx::ColorMode& colorMode,
              const gfx::CullFaceMode& cullFaceMode,
              const LayoutUniformValues& layoutUniformValues,
              const gfx::VertexBuffer<gfx::Vertex<CollisionBoxLayoutAttributes>>& layoutVertexBuffer,
              const gfx::VertexBuffer<gfx::Vertex<CollisionBoxDynamicAttributes>>& dynamicVertexBuffer,
              const gfx::IndexBuffer& indexBuffer,
              const SegmentVector<AttributeList>& segments,
              const Binders& paintPropertyBinders,
              const typename PaintProperties::PossiblyEvaluated& currentProperties,
              const TextureBindings& textureBindings,
              float currentZoom,
              const std::string& layerID) {
        UniformValues uniformValues = layoutUniformValues.concat(
            paintPropertyBinders.uniformValues(currentZoom, currentProperties));

        AttributeBindings allAttributeBindings =
            gfx::AttributeBindings<CollisionBoxLayoutAttributes>(layoutVertexBuffer)
                .concat(gfx::AttributeBindings<CollisionBoxDynamicAttributes>(dynamicVertexBuffer))
                .concat(paintPropertyBinders.attributeBindings(currentProperties));

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

} // namespace mbgl
