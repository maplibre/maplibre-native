enum {
    idFillExtrusionDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idFillExtrusionTilePropsUBO = drawableReservedUBOCount,
    idFillExtrusionPropsUBO,
    fillExtrusionUBOCount
};

struct alignas(16) FillExtrusionDrawableUBO {
    /*   0 */ float4x4 matrix;
    /*  64 */ float2 pixel_coord_upper;
    /*  72 */ float2 pixel_coord_lower;
    /*  80 */ float height_factor;
    /*  84 */ float tile_ratio;

    // Interpolations
    /*  88 */ float base_t;
    /*  92 */ float height_t;
    /*  96 */ float color_t;
    /* 100 */ float pattern_from_t;
    /* 104 */ float pattern_to_t;
    /* 108 */ float pad1;
    /* 112 */
};
static_assert(sizeof(FillExtrusionDrawableUBO) == 7 * 16, "wrong size");

struct alignas(16) FillExtrusionTilePropsUBO {
    /*  0 */ float4 pattern_from;
    /* 16 */ float4 pattern_to;
    /* 32 */ float2 texsize;
    /* 40 */ float pad1;
    /* 44 */ float pad2;
    /* 48 */
};
static_assert(sizeof(FillExtrusionTilePropsUBO) == 3 * 16, "wrong size");

/// Evaluated properties that do not depend on the tile
struct alignas(16) FillExtrusionPropsUBO {
    /*  0 */ float4 color;
    /* 16 */ float4 light_color_pad;
    /* 32 */ float4 light_position_base;
    /* 48 */ float height;
    /* 52 */ float light_intensity;
    /* 56 */ float vertical_gradient;
    /* 60 */ float opacity;
    /* 64 */ float fade;
    /* 68 */ float from_scale;
    /* 72 */ float to_scale;
    /* 76 */ float pad2;
    /* 80 */
};
static_assert(sizeof(FillExtrusionPropsUBO) == 5 * 16, "wrong size");
