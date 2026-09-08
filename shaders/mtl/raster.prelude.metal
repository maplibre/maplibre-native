enum {
    idRasterDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idRasterEvaluatedPropsUBO = drawableReservedUBOCount,
    rasterUBOCount
};

struct alignas(16) RasterDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */
};
static_assert(sizeof(RasterDrawableUBO) == 4 * 16, "wrong size");

/// Evaluated properties that do not depend on the tile
struct alignas(16) RasterEvaluatedPropsUBO {
    /*  0 */ float4 spin_weights;
    /* 16 */ float2 tl_parent;
    /* 24 */ float scale_parent;
    /* 28 */ float buffer_scale;
    /* 32 */ float fade_t;
    /* 36 */ float opacity;
    /* 40 */ float brightness_low;
    /* 44 */ float brightness_high;
    /* 48 */ float saturation_factor;
    /* 52 */ float contrast_factor;
    /* 56 */ float pad1;
    /* 60 */ float pad2;
    /* 64 */
};
static_assert(sizeof(RasterEvaluatedPropsUBO) == 4 * 16, "wrong size");
