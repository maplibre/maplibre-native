enum {
    idBackgroundDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idBackgroundPropsUBO = drawableReservedUBOCount,
    backgroundUBOCount
};

//
// Background

struct alignas(16) BackgroundDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */
};
static_assert(sizeof(BackgroundDrawableUBO) == 4 * 16, "wrong size");

/// Evaluated properties that do not depend on the tile
struct alignas(16) BackgroundPropsUBO {
    /*  0 */ float4 color;
    /* 16 */ float opacity;
    /* 20 */ float pad1;
    /* 24 */ float pad2;
    /* 28 */ float pad3;
    /* 32 */
};
static_assert(sizeof(BackgroundPropsUBO) == 2 * 16, "wrong size");

//
// Background pattern

struct alignas(16) BackgroundPatternDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float2 pixel_coord_upper;
    /* 72 */ float2 pixel_coord_lower;
    /* 80 */ float tile_units_to_pixels;
    /* 84 */ float pad1;
    /* 88 */ float pad2;
    /* 92 */ float pad3;
    /* 96 */
};
static_assert(sizeof(BackgroundPatternDrawableUBO) == 6 * 16, "wrong size");

/// Evaluated properties that do not depend on the tile
struct alignas(16) BackgroundPatternPropsUBO {
    /*  0 */ float2 pattern_tl_a;
    /*  8 */ float2 pattern_br_a;
    /* 16 */ float2 pattern_tl_b;
    /* 24 */ float2 pattern_br_b;
    /* 32 */ float2 pattern_size_a;
    /* 40 */ float2 pattern_size_b;
    /* 48 */ float scale_a;
    /* 52 */ float scale_b;
    /* 56 */ float mix;
    /* 60 */ float opacity;
    /* 64 */
};
static_assert(sizeof(BackgroundPatternPropsUBO) == 4 * 16, "wrong size");

union BackgroundDrawableUnionUBO {
    BackgroundDrawableUBO backgroundDrawableUBO;
    BackgroundPatternDrawableUBO backgroundPatternDrawableUBO;
};
