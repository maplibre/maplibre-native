enum {
    idColorReliefDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idColorReliefTilePropsUBO = drawableReservedUBOCount,
    idColorReliefEvaluatedPropsUBO,
    colorReliefUBOCount
};

struct alignas(16) ColorReliefDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */
};
static_assert(sizeof(ColorReliefDrawableUBO) == 4 * 16, "wrong size");

struct alignas(16) ColorReliefTilePropsUBO {
    /*  0 */ float4 unpack;
    /* 16 */ float2 dimension;
    /* 24 */ int32_t color_ramp_size;
    /* 28 */ float pad_tile0;
    /* 32 */
};
static_assert(sizeof(ColorReliefTilePropsUBO) == 2 * 16, "wrong size");

struct alignas(16) ColorReliefEvaluatedPropsUBO {
    /*  0 */ float opacity;
    /*  4 */ float pad_eval0;
    /*  8 */ float pad_eval1;
    /* 12 */ float pad_eval2;
    /* 16 */
};
static_assert(sizeof(ColorReliefEvaluatedPropsUBO) == 16, "wrong size");
