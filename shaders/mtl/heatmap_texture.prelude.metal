

enum {
    idHeatmapTexturePropsUBO = drawableReservedUBOCount,
    heatmapTextureUBOCount
};

struct alignas(16) HeatmapTexturePropsUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float opacity;
    /* 68 */ float pad1;
    /* 72 */ float pad2;
    /* 76 */ float pad3;
    /* 80 */
};
static_assert(sizeof(HeatmapTexturePropsUBO) == 5 * 16, "wrong size");

