enum {
    idCustomGeometryDrawableUBO = drawableReservedUBOCount,
    customGeometryUBOCount
};

struct alignas(16) CustomGeometryDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float4 color;
    /* 80 */
};
static_assert(sizeof(CustomGeometryDrawableUBO) == 5 * 16, "wrong size");
