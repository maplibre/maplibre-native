

enum {
    idHillshadeDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idHillshadeTilePropsUBO = idDrawableReservedFragmentOnlyUBO,
    idHillshadeEvaluatedPropsUBO = drawableReservedUBOCount,
    hillshadeUBOCount
};

struct alignas(16) HillshadeDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */
};
static_assert(sizeof(HillshadeDrawableUBO) == 4 * 16, "wrong size");

struct alignas(16) HillshadeTilePropsUBO {
    /*  0 */ float2 latrange;
    /*  8 */ float exaggeration;
    /* 12 */ int32_t method;
    /* 16 */ int32_t num_lights;
    /* 20 */ float pad0;
    /* 24 */ float pad1;
    /* 28 */ float pad2;
    /* 32 */
};
static_assert(sizeof(HillshadeTilePropsUBO) == 2 * 16, "wrong size");

/// Evaluated properties that do not depend on the tile
struct alignas(16) HillshadeEvaluatedPropsUBO {
    /*  0 */ float4 accent;
    /* 16 */ float4 altitudes;       // Up to 4 altitude values (in radians)
    /* 32 */ float4 azimuths;        // Up to 4 azimuth values (in radians)
    /* 48 */ float4 shadows[4];      // Shadow colors (up to 4 lights)
    /* 112 */ float4 highlights[4];  // Highlight colors (up to 4 lights)
    /* 176 */
};
static_assert(sizeof(HillshadeEvaluatedPropsUBO) == 11 * 16, "wrong size");

