enum {
    idHillshadePrepareDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idHillshadePrepareTilePropsUBO = drawableReservedUBOCount,
    hillshadePrepareUBOCount
};

struct alignas(16) HillshadePrepareDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */
};
static_assert(sizeof(HillshadePrepareDrawableUBO) == 4 * 16, "wrong size");

struct alignas(16) HillshadePrepareTilePropsUBO {
    /*  0 */ float4 unpack;
    /* 16 */ float2 dimension;
    /* 24 */ float zoom;
    /* 28 */ float maxzoom;
    /* 32 */
};
static_assert(sizeof(HillshadePrepareTilePropsUBO) == 2 * 16, "wrong size");
