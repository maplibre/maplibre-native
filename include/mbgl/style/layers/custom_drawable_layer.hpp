#pragma once

#include <mbgl/style/layer.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/change_request.hpp>
#include <mbgl/gfx/drawable_builder.hpp>

#include <array>
#include <memory>
#include <optional>

namespace mbgl {
namespace style {

class CustomDrawableLayerHost {
public:
    class Interface;

public:
    virtual ~CustomDrawableLayerHost() = default;

    virtual void initialize() = 0;

    virtual void update(Interface& interface) = 0;

    virtual void deinitialize() = 0;
};

class CustomDrawableLayerHost::Interface {
public:
    Interface(RenderLayer& layer,
              LayerGroupBasePtr& layerGroup,
              gfx::ShaderRegistry& shaders,
              gfx::Context& context,
              const TransformState& state,
              const std::shared_ptr<UpdateParameters>& updateParameters,
              const RenderTree& renderTree,
              UniqueChangeRequestVec& changes);

    std::size_t getDrawableCount() const;

    void setTileID(OverscaledTileID tileID);

    void addPolyline(const GeometryCoordinates& coordinates, const gfx::PolylineGeneratorOptions& options);

    void finish();

protected:
    gfx::ShaderPtr lineShaderDefault() const;

    std::unique_ptr<gfx::DrawableBuilder> createBuilder(const std::string& name, gfx::ShaderPtr shader) const;

private:
    RenderLayer& layer;
    LayerGroupBasePtr& layerGroup;
    gfx::ShaderRegistry& shaders;
    gfx::Context& context;
    const TransformState& state;
    const std::shared_ptr<UpdateParameters>& updateParameters;
    const RenderTree& renderTree;
    UniqueChangeRequestVec& changes;

    std::unique_ptr<gfx::DrawableBuilder> builder;
    std::optional<OverscaledTileID> tileID;
};

class CustomDrawableLayer final : public Layer {
public:
    CustomDrawableLayer(const std::string& id, std::unique_ptr<CustomDrawableLayerHost> host);

    CustomDrawableLayer(const CustomDrawableLayer&) = delete;
    ~CustomDrawableLayer() final;
    class Impl;
    const Impl& impl() const;
    Mutable<Impl> mutableImpl() const;

private:
    std::optional<conversion::Error> setPropertyInternal(const std::string& name,
                                                         const conversion::Convertible& value) final;
    StyleProperty getProperty(const std::string&) const final;
    std::unique_ptr<Layer> cloneRef(const std::string& id) const final;
    Mutable<Layer::Impl> mutableBaseImpl() const final;
};

} // namespace style
} // namespace mbgl
