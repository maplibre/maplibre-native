

enum {
    idFillDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idFillTilePropsUBO = drawableReservedUBOCount,
    idFillEvaluatedPropsUBO,
    fillUBOCount
};

//
// Fill

struct alignas(16) FillDrawableUBO {
    /*  0 */ float4x4 matrix;

    // Interpolations
    /* 64 */ float color_t;
    /* 68 */ float opacity_t;
    /* 72 */ float pad1;
    /* 76 */ float pad2;
    /* 80 */
};
static_assert(sizeof(FillDrawableUBO) == 5 * 16, "wrong size");

//
// Fill outline

struct alignas(16) FillOutlineDrawableUBO {
    /*  0 */ float4x4 matrix;

    // Interpolations
    /* 64 */ float outline_color_t;
    /* 68 */ float opacity_t;
    /* 72 */ float pad1;
    /* 76 */ float pad2;
    /* 80 */
};
static_assert(sizeof(FillOutlineDrawableUBO) == 5 * 16, "wrong size");

//
// Fill pattern

struct alignas(16) FillPatternDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float2 pixel_coord_upper;
    /* 72 */ float2 pixel_coord_lower;
    /* 80 */ float tile_ratio;

    // Interpolations
    /* 84 */ float pattern_from_t;
    /* 88 */ float pattern_to_t;
    /* 92 */ float opacity_t;
    /* 96 */
};
static_assert(sizeof(FillPatternDrawableUBO) == 6 * 16, "wrong size");

struct alignas(16) FillPatternTilePropsUBO {
    /*  0 */ float4 pattern_from;
    /* 16 */ float4 pattern_to;
    /* 32 */ float2 texsize;
    /* 40 */ float pad1;
    /* 44 */ float pad2;
    /* 48 */
};
static_assert(sizeof(FillPatternTilePropsUBO) == 3 * 16, "wrong size");

//
// Fill pattern outline

struct alignas(16) FillOutlinePatternDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float2 pixel_coord_upper;
    /* 72 */ float2 pixel_coord_lower;
    /* 80 */ float tile_ratio;

    // Interpolations
    /* 84 */ float pattern_from_t;
    /* 88 */ float pattern_to_t;
    /* 92 */ float opacity_t;
    /* 96 */
};
static_assert(sizeof(FillOutlinePatternDrawableUBO) == 6 * 16, "wrong size");

struct alignas(16) FillOutlinePatternTilePropsUBO {
    /*  0 */ float4 pattern_from;
    /* 16 */ float4 pattern_to;
    /* 32 */ float2 texsize;
    /* 40 */ float pad1;
    /* 44 */ float pad2;
    /* 48 */
};
static_assert(sizeof(FillOutlinePatternTilePropsUBO) == 3 * 16, "wrong size");

//
// Fill outline triangulated

struct alignas(16) FillOutlineTriangulatedDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float ratio;
    /* 68 */ float pad1;
    /* 72 */ float pad2;
    /* 76 */ float pad3;
    /* 80 */
};
static_assert(sizeof(FillOutlineTriangulatedDrawableUBO) == 5 * 16, "wrong size");

/// Evaluated properties that do not depend on the tile
struct alignas(16) FillEvaluatedPropsUBO {
    /*  0 */ float4 color;
    /* 16 */ float4 outline_color;
    /* 32 */ float opacity;
    /* 36 */ float fade;
    /* 40 */ float from_scale;
    /* 44 */ float to_scale;
    /* 48 */
};
static_assert(sizeof(FillEvaluatedPropsUBO) == 3 * 16, "wrong size");

union FillDrawableUnionUBO {
    FillDrawableUBO fillDrawableUBO;
    FillOutlineDrawableUBO fillOutlineDrawableUBO;
    FillPatternDrawableUBO fillPatternDrawableUBO;
    FillOutlinePatternDrawableUBO fillOutlinePatternDrawableUBO;
    FillOutlineTriangulatedDrawableUBO fillOutlineTriangulatedDrawableUBO;
};

union FillTilePropsUnionUBO {
    FillPatternTilePropsUBO fillPatternTilePropsUBO;
    FillOutlinePatternTilePropsUBO fillOutlinePatternTilePropsUBO;
};

