#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) WideVectorUniformsUBO {
    /*   0 */ std::array<float, 4 * 4> mvpMatrix;
    /*  64 */ std::array<float, 4 * 4> mvpMatrixDiff;
    /* 128 */ std::array<float, 4 * 4> mvMatrix;
    /* 192 */ std::array<float, 4 * 4> mvMatrixDiff;
    /* 256 */ std::array<float, 4 * 4> pMatrix;
    /* 320 */ std::array<float, 4 * 4> pMatrixDiff;
    /* 384 */ std::array<float, 2> frameSize;
    /* 392 */ float pad1;
    /* 396 */ float pad2;
    /* 400 */
};
static_assert(sizeof(WideVectorUniformsUBO) == 25 * 16);

struct alignas(16) WideVectorUniformWideVecUBO {
    /*  0 */ std::array<float, 4> color;
    /* 16 */ float w2;
    /* 20 */ float offset;
    /* 24 */ float edge;
    /* 28 */ float texRepeat;
    /* 32 */ std::array<float, 2> texOffset;
    /* 40 */ float miterLimit;
    /* 44 */ int32_t join;
    /* 48 */ int32_t cap;
    /* 52 */ int32_t hasExp;
    /* 56 */ float interClipLimit;
    /* 60 */ float pad1;
    /* 64 */
};
static_assert(sizeof(WideVectorUniformWideVecUBO) == 4 * 16);

struct VertexTriWideVecB {
    // x, y offset around the center
    std::array<float, 3> screenPos;
    std::array<float, 4> color;
    int index;
};
static_assert(sizeof(VertexTriWideVecB) == 32);

struct VertexTriWideVecInstance {
    std::array<float, 4> center;
    std::array<float, 4> color;
    int32_t prev;
    int32_t next;
    int64_t pad1;
};
static_assert(sizeof(VertexTriWideVecInstance) == 48);

} // namespace shaders
} // namespace mbgl
