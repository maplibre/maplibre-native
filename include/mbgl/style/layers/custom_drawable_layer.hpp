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
    /// @brief Construct a new Interface object (internal core use only)
    Interface(RenderLayer& layer,
              LayerGroupBasePtr& layerGroup,
              gfx::ShaderRegistry& shaders,
              gfx::Context& context,
              const TransformState& state,
              const std::shared_ptr<UpdateParameters>& updateParameters,
              const RenderTree& renderTree,
              UniqueChangeRequestVec& changes);
    /**
     * @brief Get the drawable count
     *
     * @return std::size_t
     */
    std::size_t getDrawableCount() const;

    /**
     * @brief Set the Tile ID
     *
     * @param tileID
     */
    void setTileID(OverscaledTileID tileID);
    
    void setColor(Color color);

    /**
     * @brief Add a polyline
     *
     * @param coordinates
     * @param options Polyline options
     */
    void addPolyline(const GeometryCoordinates& coordinates, const gfx::PolylineGeneratorOptions& options);

    /**
     * @brief Finishe the current drawable building session
     *
     */
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
    std::optional<Color> currentColor;
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
