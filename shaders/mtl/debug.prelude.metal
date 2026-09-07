

enum {
    idDebugUBO = drawableReservedUBOCount,
    debugUBOCount
};

struct alignas(16) DebugUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float4 color;
    /* 80 */ float overlay_scale;
    /* 84 */ float pad1;
    /* 88 */ float pad2;
    /* 92 */ float pad3;
    /* 96 */
};
static_assert(sizeof(DebugUBO) == 6 * 16, "wrong size");

