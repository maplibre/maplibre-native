#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) WideVectorUniformsUBO {
    std::array<float, 4 * 4> mvpMatrix;
    std::array<float, 4 * 4> mvpMatrixDiff;
    std::array<float, 4 * 4> mvMatrix;
    std::array<float, 4 * 4> mvMatrixDiff;
    std::array<float, 4 * 4> pMatrix;
    std::array<float, 4 * 4> pMatrixDiff;
    std::array<float, 2> frameSize;
};
static_assert(sizeof(WideVectorUniformsUBO) % 16 == 0);

struct alignas(16) WideVectorUniformWideVecUBO {
    std::array<float, 4> color;
    float w2;
    float offset;
    float edge;
    float texRepeat;
    std::array<float, 2> texOffset;
    float miterLimit;
    int32_t join;
    int32_t cap;
    int32_t hasExp;
    float interClipLimit;
};
static_assert(sizeof(WideVectorUniformWideVecUBO) % 16 == 0);

enum {
    idWideVectorUniformsUBO = globalUBOCount,
    idWideVectorUniformWideVecUBO,
    wideVectorUBOCount
};

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
    int64_t pad_;
};
static_assert(sizeof(VertexTriWideVecInstance) == 48);

} // namespace shaders
} // namespace mbgl
