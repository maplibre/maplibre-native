#include <mbgl/gfx/compute_pass.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/util/projection.hpp>

namespace mbgl {
namespace gfx {

using namespace shaders;

float pixelsToTileUnits(const float pixelValue, const uint16_t tileIdCanonicalZ, const float zoom) {
    return pixelValue * (static_cast<float>(util::EXTENT) / (static_cast<float>(util::tileSize_D) * std::pow(2.f, zoom - tileIdCanonicalZ)));
}

mat4 computeTileMatrix(SymbolComputeUBO& computeUBO) {
    // from RenderTile::prepare
    mat4 tileMatrix;
    const uint64_t tileScale = 1ull << computeUBO.tileIdCanonicalZ;
    const double s = Projection::worldSize(computeUBO.scale) / tileScale;
    
    matrix::identity(tileMatrix);
    matrix::translate(tileMatrix,
                      tileMatrix,
                      int64_t(computeUBO.tileIdCanonicalX + computeUBO.tileIdWrap * static_cast<int64_t>(tileScale)) * s,
                      int64_t(computeUBO.tileIdCanonicalY) * s,
                      0);
    matrix::scale(tileMatrix, tileMatrix, s / util::EXTENT, s / util::EXTENT, 1);
    
    computeUBO.projMatrix[14] -= ((1 + computeUBO.layerIndex) * PaintParameters::numSublayers - computeUBO.subLayerIndex) * PaintParameters::depthEpsilon;
    matrix::multiply(tileMatrix, util::cast<double>(computeUBO.projMatrix), tileMatrix);
    
    if (computeUBO.translation[0] == 0 && computeUBO.translation[1] == 0) {
        return tileMatrix;
    }
    
    mat4 vtxMatrix;
    
    const float angle = computeUBO.inViewportPixelUnits
    ? (computeUBO.isAnchorMap ? static_cast<float>(computeUBO.bearing) : 0.0f)
    : (computeUBO.isAnchorMap ? 0.0f : static_cast<float>(-computeUBO.bearing));
    
    Point<float> translate = util::rotate(Point<float>{computeUBO.translation[0], computeUBO.translation[1]}, angle);
    
    if (computeUBO.inViewportPixelUnits) {
        matrix::translate(vtxMatrix, tileMatrix, translate.x, translate.y, 0);
    } else {
        matrix::translate(vtxMatrix,
                          tileMatrix,
                          pixelsToTileUnits(translate.x, computeUBO.tileIdCanonicalZ, static_cast<float>(computeUBO.zoom)),
                          pixelsToTileUnits(translate.y, computeUBO.tileIdCanonicalZ, static_cast<float>(computeUBO.zoom)),
                          0);
    }
    
    return vtxMatrix;
}

mat4 computeLabelPlaneMatrix(const mat4& posMatrix,
                             const SymbolComputeUBO& computeUBO,
                             const float pixelsToTileUnits) {
    if (computeUBO.alongLine || computeUBO.hasVariablePlacement) {
        return matrix::identity4();
    }
    
    mat4 m;
    matrix::identity(m);
    if (computeUBO.pitchWithMap) {
        matrix::scale(m, m, 1 / pixelsToTileUnits, 1 / pixelsToTileUnits, 1);
        if (!computeUBO.rotateWithMap) {
            matrix::rotate_z(m, m, computeUBO.bearing);
        }
    } else {
        matrix::scale(m, m, computeUBO.width / 2.0, -(computeUBO.height / 2.0), 1.0);
        matrix::translate(m, m, 1, -1, 0);
        matrix::multiply(m, m, posMatrix);
    }
    return m;
}

mat4 computeGlCoordMatrix(const mat4& posMatrix,
                          const SymbolComputeUBO& computeUBO,
                          const float pixelsToTileUnits) {
    mat4 m;
    matrix::identity(m);
    if (computeUBO.pitchWithMap) {
        matrix::multiply(m, m, posMatrix);
        matrix::scale(m, m, pixelsToTileUnits, pixelsToTileUnits, 1);
        if (!computeUBO.rotateWithMap) {
            matrix::rotate_z(m, m, -computeUBO.bearing);
        }
    } else {
        matrix::scale(m, m, 1, -1, 1);
        matrix::translate(m, m, -1, -1, 0);
        matrix::scale(m, m, 2.0 / computeUBO.width, 2.0 / computeUBO.height, 1.0);
    }
    return m;
}

void computeDrawableUBOElement(std::vector<SymbolDrawableUBO>& out, std::vector<SymbolComputeUBO>& in, size_t i) {
    auto& computeUBO = in[i];
    
    const auto matrix = computeTileMatrix(computeUBO);
    
    // from symbol_program, makeValues
    const auto currentZoom = static_cast<float>(computeUBO.zoom);
    const float pixelsToUnits = pixelsToTileUnits(1.f, computeUBO.tileIdCanonicalZ, currentZoom);
    
    const mat4 labelPlaneMatrix = computeLabelPlaneMatrix(matrix, computeUBO, pixelsToUnits);
    const mat4 glCoordMatrix = computeGlCoordMatrix(matrix, computeUBO, pixelsToUnits);
    
    const float gammaScale = (computeUBO.pitchWithMap ? static_cast<float>(std::cos(computeUBO.pitch)) * computeUBO.camDist : 1.0f);
    
    // Line label rotation happens in `updateLineLabels`/`reprojectLineLabels``
    // Pitched point labels are automatically rotated by the labelPlaneMatrix projection
    // Unpitched point labels need to have their rotation applied after projection
    const bool rotateInShader = computeUBO.rotateWithMap && !computeUBO.pitchWithMap && !computeUBO.alongLine;
    
    out[i] = {
        /*.matrix=*/util::cast<float>(matrix),
        /*.label_plane_matrix=*/util::cast<float>(labelPlaneMatrix),
        /*.coord_matrix=*/util::cast<float>(glCoordMatrix),
        
        /*.texsize=*/computeUBO.texsize,
        /*.texsize_icon=*/computeUBO.texsize_icon,
        
        /*.gamma_scale=*/gammaScale,
        /*.rotate_symbol=*/rotateInShader,
        /*.pad=*/{0},
    };
}

ComputePass::ComputePass(gfx::Context& _context)
    : context(_context) {}

void ComputePass::computeDrawableBuffer(std::vector<SymbolComputeUBO>& computeUBOVector,
                                        [[maybe_unused]] gfx::UniformBufferPtr& computeBuffer,
                                        gfx::UniformBufferPtr& drawableBuffer) {
    std::vector<SymbolDrawableUBO> drawableUBOVector(computeUBOVector.size());
    for(size_t i = 0; i < computeUBOVector.size(); i++) {
        computeDrawableUBOElement(drawableUBOVector, computeUBOVector, i);
    }

    const size_t drawableUBOVectorSize = sizeof(SymbolDrawableUBO) * drawableUBOVector.size();
    if (!drawableBuffer || drawableBuffer->getSize() < drawableUBOVectorSize) {
        drawableBuffer = context.createUniformBuffer(drawableUBOVector.data(), drawableUBOVectorSize);
    } else {
        drawableBuffer->update(drawableUBOVector.data(), drawableUBOVectorSize);
    }
}

} // namespace gfx
} // namespace mbgl
