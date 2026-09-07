

enum {
    idCircleDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idCircleEvaluatedPropsUBO = drawableReservedUBOCount,
    circleUBOCount
};

struct alignas(16) CircleDrawableUBO {
    /*   0 */ float4x4 matrix;
    /*  64 */ float2 extrude_scale;

    // Interpolations
    /*  72 */ float color_t;
    /*  76 */ float radius_t;
    /*  80 */ float blur_t;
    /*  84 */ float opacity_t;
    /*  88 */ float stroke_color_t;
    /*  92 */ float stroke_width_t;
    /*  96 */ float stroke_opacity_t;
    /* 100 */ float pad1;
    /* 104 */ float pad2;
    /* 108 */ float pad3;
    /* 112 */
};
static_assert(sizeof(CircleDrawableUBO) == 7 * 16, "wrong size");

/// Evaluated properties that do not depend on the tile
struct alignas(16) CircleEvaluatedPropsUBO {
    /*  0 */ float4 color;
    /* 16 */ float4 stroke_color;
    /* 32 */ float radius;
    /* 36 */ float blur;
    /* 40 */ float opacity;
    /* 44 */ float stroke_width;
    /* 48 */ float stroke_opacity;
    /* 52 */ int scale_with_map;
    /* 56 */ int pitch_with_map;
    /* 60 */ float pad1;
    /* 64 */
};
static_assert(sizeof(CircleEvaluatedPropsUBO) == 4 * 16, "wrong size");

