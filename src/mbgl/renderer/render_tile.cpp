#include <mbgl/renderer/render_tile.hpp>

#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/renderer/buckets/debug_bucket.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/tile_render_data.hpp>
#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/math.hpp>

namespace mbgl {

using namespace style;

RenderTile::RenderTile(UnwrappedTileID id_, Tile& tile_)
    : id(id_),
      tile(tile_) {}

RenderTile::~RenderTile() = default;

mat4 RenderTile::translateVtxMatrix(const UnwrappedTileID& id,
                                    const mat4& tileMatrix,
                                    const std::array<float, 2>& translation,
                                    TranslateAnchorType anchor,
                                    const TransformState& state,
                                    const bool inViewportPixelUnits) {
    if (translation[0] == 0 && translation[1] == 0) {
        return tileMatrix;
    }

    mat4 vtxMatrix;

    const float angle = inViewportPixelUnits
                            ? (anchor == TranslateAnchorType::Map ? static_cast<float>(state.getBearing()) : 0.0f)
                            : (anchor == TranslateAnchorType::Viewport ? static_cast<float>(-state.getBearing())
                                                                       : 0.0f);

    Point<float> translate = util::rotate(Point<float>{translation[0], translation[1]}, angle);

    if (inViewportPixelUnits) {
        matrix::translate(vtxMatrix, tileMatrix, translate.x, translate.y, 0);
    } else {
        matrix::translate(vtxMatrix,
                          tileMatrix,
                          id.pixelsToTileUnits(translate.x, static_cast<float>(state.getZoom())),
                          id.pixelsToTileUnits(translate.y, static_cast<float>(state.getZoom())),
                          0);
    }

    return vtxMatrix;
}

mat4 RenderTile::translateVtxMatrix(const mat4& tileMatrix,
                                    const std::array<float, 2>& translation,
                                    TranslateAnchorType anchor,
                                    const TransformState& state,
                                    const bool inViewportPixelUnits) const {
    return translateVtxMatrix(id, tileMatrix, translation, anchor, state, inViewportPixelUnits);
}

mat4 RenderTile::translatedMatrix(const std::array<float, 2>& translation,
                                  TranslateAnchorType anchor,
                                  const TransformState& state) const {
    return translateVtxMatrix(matrix, translation, anchor, state, false);
}

mat4 RenderTile::translatedClipMatrix(const std::array<float, 2>& translation,
                                      TranslateAnchorType anchor,
                                      const TransformState& state) const {
    return translateVtxMatrix(nearClippedMatrix, translation, anchor, state, false);
}

const OverscaledTileID& RenderTile::getOverscaledTileID() const {
    return tile.id;
}
bool RenderTile::holdForFade() const {
    return tile.holdForFade();
}

Bucket* RenderTile::getBucket(const style::Layer::Impl& impl) const {
    assert(renderData);
    return renderData->getBucket(impl);
}

const LayerRenderData* RenderTile::getLayerRenderData(const style::Layer::Impl& impl) const {
    assert(renderData);
    return renderData->getLayerRenderData(impl);
}

std::optional<ImagePosition> RenderTile::getPattern(const std::string& pattern) const {
    assert(renderData);
    return renderData->getPattern(pattern);
}

static const std::shared_ptr<TileAtlasTextures> noAtlas;
const std::shared_ptr<TileAtlasTextures>& RenderTile::getAtlasTextures() const {
    return renderData ? renderData->getAtlasTextures() : noAtlas;
}

void RenderTile::upload(gfx::UploadPass& uploadPass) const {
    assert(renderData);
    renderData->upload(uploadPass);

    if (debugBucket) {
        debugBucket->upload(uploadPass);
    }
}

void RenderTile::prepare(const SourcePrepareParameters& parameters) {
    renderData = tile.createRenderData();
    assert(renderData);
    renderData->prepare(parameters);

    needsRendering = tile.usedByRenderedLayers;

    if (parameters.debugOptions != MapDebugOptions::NoDebug &&
        (!debugBucket || debugBucket->renderable != tile.isRenderable() || debugBucket->complete != tile.isComplete() ||
         !(debugBucket->modified == tile.modified) || !(debugBucket->expires == tile.expires) ||
         debugBucket->debugMode != parameters.debugOptions)) {
        debugBucket = std::make_unique<DebugBucket>(
            tile.id, tile.isRenderable(), tile.isComplete(), tile.modified, tile.expires, parameters.debugOptions);
    } else if (parameters.debugOptions == MapDebugOptions::NoDebug) {
        debugBucket.reset();
    }

    // Calculate two matrices for this tile: matrix is the standard tile matrix;
    // nearClippedMatrix has near plane moved further, to enhance depth buffer
    // precision
    const auto& transform = parameters.transform;
    transform.state.matrixFor(matrix, id);
    transform.state.matrixFor(nearClippedMatrix, id);
    matrix::multiply(matrix, transform.projMatrix, matrix);
    matrix::multiply(nearClippedMatrix, transform.nearClippedProjMatrix, nearClippedMatrix);
}

void RenderTile::setFeatureState(const LayerFeatureStates& states) {
    tile.setFeatureState(states);
}

} // namespace mbgl
