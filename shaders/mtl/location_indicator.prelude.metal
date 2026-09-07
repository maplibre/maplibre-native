

enum {
    idLocationIndicatorUBO = drawableReservedUBOCount,
    locationIndicatorUBOCount
};

struct alignas(16) LocationIndicatorDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float4 color;
    /* 80 */
};
static_assert(sizeof(LocationIndicatorDrawableUBO) == 5 * 16, "wrong size");

