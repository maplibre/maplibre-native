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
    enum class LineShaderType {
        Classic,
        WideVector
    };

    struct LineOptions {
        gfx::PolylineGeneratorOptions geometry;

        float blur = 0.f;
        float opacity = 1.f;
        float gapWidth = 0.f;
        float offset = 0.f;
        float width = 1.f;
        Color color;
    };

    struct FillOptions {
        Color color;
        float opacity = 1.f;
    };

    struct SymbolOptions {
        Size size;
        gfx::Texture2DPtr texture;
        std::array<float, 2> anchor{0.5f, 0.5f};
        float angleDegrees{.0f};
        bool scaleWithMap{false};
        bool pitchWithMap{false};
    };

    struct GeometryVertex {
        std::array<float, 3> position;
        std::array<float, 2> texcoords;
    };

    struct GeometryOptions {
        mat4 matrix = matrix::identity4();
        Color color = Color::white();
        gfx::Texture2DPtr texture;
    };

    template <typename T>
    using TweakerCallback = std::function<void(mbgl::gfx::Drawable&, const mbgl::PaintParameters&, T&)>;

    using LineTweakerCallback = TweakerCallback<LineOptions>;
    using FillTweakerCallback = TweakerCallback<FillOptions>;
    using SymbolTweakerCallback = TweakerCallback<SymbolOptions>;
    using GeometryTweakerCallback = TweakerCallback<GeometryOptions>;

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
     * @brief Set the geometry options
     *
     * @param options
     */
    void setGeometryOptions(const GeometryOptions& options);

    void setLineTweakerCallback(LineTweakerCallback&& callback) { lineTweakerCallback = callback; }
    void setFillTweakerCallback(FillTweakerCallback&& callback) { fillTweakerCallback = callback; }
    void setSymbolTweakerCallback(SymbolTweakerCallback&& callback) { symbolTweakerCallback = callback; }
    void setGeometryTweakerCallback(GeometryTweakerCallback&& callback) { geometryTweakerCallback = callback; }

    /**
     * @brief Add a polyline
     *
     * @param coordinates in tile range
     * @param shaderType
     * @return a valid util::SimpleIdentity if the polyline was added
     */
    util::SimpleIdentity addPolyline(const GeometryCoordinates& coordinates,
                                     LineShaderType shaderType = LineShaderType::Classic);

    /**
     * @brief Add a polyline
     *
     * @param coordinates Geographic coordinates
     * @param shaderType
     * @return a valid util::SimpleIdentity if the poline was added
     */
    util::SimpleIdentity addPolyline(const LineString<double>& coordinates,
                                     LineShaderType shaderType = LineShaderType::Classic);

    /**
     * @brief Add a multipolygon area fill
     *
     * @param geometry a collection of rings with optional holes
     * @return a valid util::SimpleIdentity if the fill was added
     */
    util::SimpleIdentity addFill(const GeometryCollection& geometry);

    /**
     * @brief Add a symbol
     *
     * @param point
     * @param textureCoordinates (optional mapping)
     * @return a valid util::SimpleIdentity if the symbol was added
     */
    util::SimpleIdentity addSymbol(const GeometryCoordinate& point,
                                   const std::array<std::array<float, 2>, 2>& textureCoordinates = {{{0, 0}, {1, 1}}});

    util::SimpleIdentity addGeometry(std::shared_ptr<gfx::VertexVector<GeometryVertex>> vertices,
                                     std::shared_ptr<gfx::IndexVector<gfx::Triangles>> indices,
                                     bool is3D);

    /**
     * @brief Finish the current drawable building session
     *
     */
    void finish();

    void removeDrawable(const util::SimpleIdentity& id);

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
    gfx::ShaderPtr lineShaderWideVector() const;
    gfx::ShaderPtr fillShaderDefault() const;
    gfx::ShaderPtr symbolShaderDefault() const;
    gfx::ShaderPtr geometryShaderDefault() const;

    enum class BuilderType {
        None,
        LineClassic,
        LineWideVector,
        Fill,
        Symbol,
        Geometry
    };

    std::unique_ptr<gfx::DrawableBuilder> createBuilder(const std::string& name, gfx::ShaderPtr shader) const;
    bool updateBuilder(BuilderType type, const std::string& name, gfx::ShaderPtr shader);

    std::unique_ptr<gfx::DrawableBuilder> builder;
    std::optional<OverscaledTileID> tileID;

    LineOptions lineOptions;
    FillOptions fillOptions;
    SymbolOptions symbolOptions;
    GeometryOptions geometryOptions;

    LineTweakerCallback lineTweakerCallback;
    FillTweakerCallback fillTweakerCallback;
    SymbolTweakerCallback symbolTweakerCallback;
    GeometryTweakerCallback geometryTweakerCallback;

    BuilderType builderType{BuilderType::None};
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
