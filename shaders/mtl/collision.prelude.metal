

enum {
    idCollisionDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idCollisionTilePropsUBO = drawableReservedUBOCount,
    collisionUBOCount
};

struct alignas(16) CollisionDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */
};
static_assert(sizeof(CollisionDrawableUBO) == 4 * 16, "wrong size");

struct alignas(16) CollisionTilePropsUBO {
    /*  0 */ float2 extrude_scale;
    /*  8 */ float overscale_factor;
    /* 12 */ float pad1;
    /* 16 */
};
static_assert(sizeof(CollisionTilePropsUBO) == 16, "wrong size");

