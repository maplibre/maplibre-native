#include <mbgl/renderer/sources/render_tile_source.hpp>

#include <mbgl/renderer/buckets/debug_bucket.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/renderer/tile_render_data.hpp>
#include <mbgl/tile/vector_tile.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/math.hpp>

#if MLN_DRAWABLE_RENDERER
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/util/convert.hpp>

#include <unordered_set>
#endif

namespace mbgl {

using namespace style;

void TileSourceRenderItem::upload(gfx::UploadPass& parameters) const {
    for (auto& tile : *renderTiles) {
        tile.upload(parameters);
    }
}

void TileSourceRenderItem::render(PaintParameters& parameters) const {
    for (auto& tile : *renderTiles) {
        tile.finishRender(parameters);
    }
}

#if MLN_DRAWABLE_RENDERER
void TileSourceRenderItem::updateDebugDrawables(LayerGroupBasePtr layerGroup, PaintParameters& parameters) const {
    TileLayerGroup* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());
    if (parameters.debugOptions & (MapDebugOptions::TileBorders)) {
        auto& context = parameters.context;
        auto& shaders = *parameters.staticData.shaders;
        constexpr auto DebugShaderName = "DebugShader";
        gfx::ShaderProgramBasePtr debugShader = context.getGenericShader(shaders, std::string(DebugShaderName));
        const auto renderPass = RenderPass::None;

        auto createBuilder = [&context](const std::string& builderName,
                                        gfx::ShaderProgramBasePtr shader) -> std::unique_ptr<gfx::DrawableBuilder> {
            constexpr auto VertexAttribName = "a_pos";

            std::unique_ptr<gfx::DrawableBuilder> builder = context.createDrawableBuilder(builderName);
            builder->setShader(shader);
            builder->setRenderPass(renderPass);
            builder->setEnableDepth(false);
            builder->setColorMode(gfx::ColorMode::unblended());
            builder->setCullFaceMode(gfx::CullFaceMode::disabled());
            builder->setEnableStencil(false);
            builder->setVertexAttrName(VertexAttribName);

            return builder;
        };

        struct alignas(16) DebugUBO {
            std::array<float, 4 * 4> matrix;
            Color color;
            float overlay_scale;
            float pad1, pad2, pad3;
        };
        static_assert(sizeof(DebugUBO) % 16 == 0);
        constexpr auto DebugUBOName = "DebugUBO";

        // create texture. to be reused for all the tiles of the layer
        std::array<uint8_t, 4> data{{0, 0, 0, 0}};
        auto emptyImage = std::make_shared<PremultipliedImage>(Size(1, 1), data.data(), data.size());
        auto texture = context.createTexture2D();
        texture->setImage(emptyImage);
        texture->setSamplerConfiguration(
            {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
        constexpr auto DebugOverlayUniformName = "u_overlay";
        const auto samplerLocation = debugShader->getSamplerLocation(DebugOverlayUniformName);

        // erase drawables that are not in the current tile set
        std::unordered_set<OverscaledTileID> newTiles;
        for (auto& tile : *renderTiles) {
            if (tile.getNeedsRendering()) {
                newTiles.insert(tile.getOverscaledTileID());
            }
        }
        tileLayerGroup->observeDrawablesRemove([&](gfx::Drawable& drawable) {
            return (drawable.getTileID().has_value() && newTiles.count(*drawable.getTileID()) > 0);
        });

        // add new drawables and update existing ones
        auto builder = createBuilder("debug", debugShader);
        for (auto& tile : *renderTiles) {
            if (tile.getNeedsRendering()) {
                const auto tileID = tile.getOverscaledTileID();
                auto& debugBucket = tile.debugBucket;
                const DebugUBO debugUBO{/*matrix = */ util::cast<float>(tile.matrix),
                                        /*color = */ Color::red(),
                                        /*overlay_scale = */ 1.0f,
                                        0,
                                        0,
                                        0};
                auto updatedCount = tileLayerGroup->observeDrawables(renderPass, tileID, [&](gfx::Drawable& drawable) {
                    // update existing drawable
                    auto& uniforms = drawable.mutableUniformBuffers();
                    uniforms.createOrUpdate(DebugUBOName, &debugUBO, context);
                });

                if (0 == updatedCount) {
                    // create new drawable
                    if (debugBucket && samplerLocation.has_value()) {
                        auto vertices = RenderStaticData::tileVertices().vector();
                        auto indexes = RenderStaticData::tileLineStripIndices().vector();
                        auto segments = RenderStaticData::tileBorderSegments();

                        std::vector<std::array<int16_t, 2>> verts(vertices.size());
                        std::transform(vertices.begin(),
                                       vertices.end(),
                                       verts.begin(),
                                       [](const auto& v) -> std::array<int16_t, 2> { return v.a1; });

                        builder->addVertices(verts, 0, verts.size());
                        builder->setSegments(
                            gfx::LineStrip(4.0f * parameters.pixelRatio), indexes, segments.data(), segments.size());
                        // texture
                        builder->setTexture(texture, samplerLocation.value());

                        // finish
                        builder->flush();
                        for (auto& drawable : builder->clearDrawables()) {
                            drawable->setTileID(tileID);
                            auto& uniforms = drawable->mutableUniformBuffers();
                            uniforms.createOrUpdate(DebugUBOName, &debugUBO, context);

                            tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                        }
                    }
                }
            }
        }
    } else {
        tileLayerGroup->clearDrawables();
    }
}
#endif

RenderTileSource::RenderTileSource(Immutable<style::Source::Impl> impl_)
    : RenderSource(std::move(impl_)),
      renderTiles(makeMutable<std::vector<RenderTile>>()) {
    tilePyramid.setObserver(this);
}

RenderTileSource::~RenderTileSource() = default;

bool RenderTileSource::isLoaded() const {
    return tilePyramid.isLoaded();
}

std::unique_ptr<RenderItem> RenderTileSource::createRenderItem() {
    return std::make_unique<TileSourceRenderItem>(renderTiles, baseImpl->id);
}

void RenderTileSource::prepare(const SourcePrepareParameters& parameters) {
    bearing = static_cast<float>(parameters.transform.state.getBearing());
    filteredRenderTiles = nullptr;
    renderTilesSortedByY = nullptr;
    auto tiles = makeMutable<std::vector<RenderTile>>();
    tiles->reserve(tilePyramid.getRenderedTiles().size());
    for (auto& entry : tilePyramid.getRenderedTiles()) {
        tiles->emplace_back(entry.first, entry.second);
        tiles->back().prepare(parameters);
    }
    featureState.coalesceChanges(*tiles);
    renderTiles = std::move(tiles);
}

void RenderTileSource::updateFadingTiles() {
    tilePyramid.updateFadingTiles();
}

bool RenderTileSource::hasFadingTiles() const {
    return tilePyramid.hasFadingTiles();
}

RenderTiles RenderTileSource::getRenderTiles() const {
    if (!filteredRenderTiles) {
        auto result = std::make_shared<std::vector<std::reference_wrapper<const RenderTile>>>();
        for (const auto& renderTile : *renderTiles) {
            if (renderTile.holdForFade()) {
                continue;
            }
            result->emplace_back(renderTile);
        }
        filteredRenderTiles = std::move(result);
    }
    return filteredRenderTiles;
}

RenderTiles RenderTileSource::getRenderTilesSortedByYPosition() const {
    if (!renderTilesSortedByY) {
        const auto comp = [bearing = this->bearing](const RenderTile& a, const RenderTile& b) {
            Point<float> pa(static_cast<float>(a.id.canonical.x), static_cast<float>(a.id.canonical.y));
            Point<float> pb(static_cast<float>(b.id.canonical.x), static_cast<float>(b.id.canonical.y));

            auto par = util::rotate(pa, bearing);
            auto pbr = util::rotate(pb, bearing);

            return std::tie(b.id.canonical.z, par.y, par.x) < std::tie(a.id.canonical.z, pbr.y, pbr.x);
        };

        auto result = std::make_shared<std::vector<std::reference_wrapper<const RenderTile>>>();
        result->reserve(renderTiles->size());
        for (const auto& renderTile : *renderTiles) {
            result->emplace_back(renderTile);
        }
        std::sort(result->begin(), result->end(), comp);
        renderTilesSortedByY = std::move(result);
    }
    return renderTilesSortedByY;
}

const Tile* RenderTileSource::getRenderedTile(const UnwrappedTileID& tileID) const {
    return tilePyramid.getRenderedTile(tileID);
}

std::unordered_map<std::string, std::vector<Feature>> RenderTileSource::queryRenderedFeatures(
    const ScreenLineString& geometry,
    const TransformState& transformState,
    const std::unordered_map<std::string, const RenderLayer*>& layers,
    const RenderedQueryOptions& options,
    const mat4& projMatrix) const {
    return tilePyramid.queryRenderedFeatures(geometry, transformState, layers, options, projMatrix, featureState);
}

std::vector<Feature> RenderTileSource::querySourceFeatures(const SourceQueryOptions& options) const {
    return tilePyramid.querySourceFeatures(options);
}

void RenderTileSource::setFeatureState(const std::optional<std::string>& sourceLayerID,
                                       const std::string& featureID,
                                       const FeatureState& state) {
    featureState.updateState(sourceLayerID, featureID, state);
}

void RenderTileSource::getFeatureState(FeatureState& state,
                                       const std::optional<std::string>& sourceLayerID,
                                       const std::string& featureID) const {
    featureState.getState(state, sourceLayerID, featureID);
}

void RenderTileSource::removeFeatureState(const std::optional<std::string>& sourceLayerID,
                                          const std::optional<std::string>& featureID,
                                          const std::optional<std::string>& stateKey) {
    featureState.removeState(sourceLayerID, featureID, stateKey);
}

void RenderTileSource::reduceMemoryUse() {
    tilePyramid.reduceMemoryUse();
}

void RenderTileSource::dumpDebugLogs() const {
    tilePyramid.dumpDebugLogs();
}

// RenderTileSetSource implementation

RenderTileSetSource::RenderTileSetSource(Immutable<style::Source::Impl> impl_)
    : RenderTileSource(std::move(impl_)) {}

RenderTileSetSource::~RenderTileSetSource() = default;

uint8_t RenderTileSetSource::getMaxZoom() const {
    return cachedTileset ? cachedTileset->zoomRange.max : util::TERRAIN_RGB_MAXZOOM;
}

void RenderTileSetSource::update(Immutable<style::Source::Impl> baseImpl_,
                                 const std::vector<Immutable<style::LayerProperties>>& layers,
                                 const bool needsRendering,
                                 const bool needsRelayout,
                                 const TileParameters& parameters) {
    std::swap(baseImpl, baseImpl_);

    enabled = needsRendering;

    const auto& implTileset = getTileset();
    // In Continuous mode, keep the existing tiles if the new cachedTileset is
    // not yet available, thus providing smart style transitions without
    // flickering. In other modes, allow clearing the tile pyramid first, before
    // the early return in order to avoid render tests being flaky.
    bool canUpdateTileset = implTileset || parameters.mode != MapMode::Continuous;
    if (canUpdateTileset && cachedTileset != implTileset) {
        cachedTileset = implTileset;

        // TODO: this removes existing buckets, and will cause flickering.
        // Should instead refresh tile data in place.
        tilePyramid.clearAll();
    }

    if (!cachedTileset) return;

    updateInternal(*cachedTileset, layers, needsRendering, needsRelayout, parameters);
}

} // namespace mbgl
