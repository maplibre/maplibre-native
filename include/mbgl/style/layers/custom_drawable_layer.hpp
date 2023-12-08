#pragma once

#include <mbgl/style/layer.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/change_request.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/gfx/context.hpp>

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
    struct LineOptions {
        Color color;
        float blur = 0.f;
        float opacity = 1.f;
        float gapWidth = 0.f;
        float offset = 0.f;
        float width = 1.f;
        gfx::PolylineGeneratorOptions geometry;
    };

    struct FillOptions {
        Color color;
        float opacity = 1.f;
    };

    struct SymbolOptions {
        Color color;
        Size size;
        gfx::Texture2DPtr texture;
    };

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

    /**
     * @brief Set the line options
     *
     * @param options
     */
    void setLineOptions(const LineOptions& options);

    /**
     * @brief Set the fill options
     *
     * @param options
     */
    void setFillOptions(const FillOptions& options);

    /**
     * @brief Set the Symbol options
     *
     * @param options
     */
    void setSymbolOptions(const SymbolOptions& options);

    /**
     * @brief Add a polyline
     *
     * @param coordinates
     * @param options Polyline options
     */
    void addPolyline(const GeometryCoordinates& coordinates);

    /**
     * @brief Add a multipolygon area fill
     *
     * @param geometry a collection of rings with optional holes
     */
    void addFill(const GeometryCollection& geometry);

    /**
     * @brief Add a symbol
     *
     * @param point
     */
    void addSymbol(const GeometryCoordinate& point);

    /**
     * @brief Finish the current drawable building session
     *
     */
    void finish();

public:
    RenderLayer& layer;
    LayerGroupBasePtr& layerGroup;
    gfx::ShaderRegistry& shaders;
    gfx::Context& context;
    const TransformState& state;
    const std::shared_ptr<UpdateParameters>& updateParameters;
    const RenderTree& renderTree;
    UniqueChangeRequestVec& changes;

private:
    gfx::ShaderPtr lineShaderDefault() const;
    gfx::ShaderPtr fillShaderDefault() const;
    gfx::ShaderPtr symbolShaderDefault() const;

    std::unique_ptr<gfx::DrawableBuilder> createBuilder(const std::string& name, gfx::ShaderPtr shader) const;

    std::unique_ptr<gfx::DrawableBuilder> builder;
    std::optional<OverscaledTileID> tileID;

    gfx::ShaderPtr lineShader;
    gfx::ShaderPtr fillShader;
    gfx::ShaderPtr symbolShader;

    LineOptions lineOptions;
    FillOptions fillOptions;
    SymbolOptions symbolOptions;
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
