

enum {
    idClippingMaskUBO = idDrawableReservedVertexOnlyUBO,
    clippingMaskUBOCount = drawableReservedUBOCount
};

struct alignas(16) ClipUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ uint32_t stencil_ref;
    /* 68 */ float pad1;
    /* 72 */ float pad2;
    /* 76 */ float pad3;
    /* 80 */
};
static_assert(sizeof(ClipUBO) == 5 * 16, "wrong size");

