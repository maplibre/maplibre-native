#pragma once

#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/vulkan/shader_program.hpp>

namespace mln {
namespace shaders {

constexpr auto fillShaderPrelude = R"(

#define idFillDrawableUBO           idDrawableReservedVertexOnlyUBO
#define idFillTilePropsUBO          drawableReservedUBOCount
#define idFillEvaluatedPropsUBO     layerUBOStartId
#define idFillPosVertexDrawableUBO        drawableSSBOStartId
#define idFillPrevNextVertexDrawableUBO   drawableSSBOStartId + 1
#define idFillDataVertexDrawableUBO       drawableSSBOStartId + 2

#if !defined(HAS_UNIFORM_u_triangle_instance)
float sdCircle(vec2 p, float r) {
    return length(p) - r;
}

float sdPlane(vec2 p, vec2 n, float h) {
    // n must be normalized
    return dot(p, n) + h;
}

float dot2(vec2 v) {
    return dot(v, v);
}

float opUnion(float a, float b) {
    return min(a, b);
}

float opSmoothUnion(float d1, float d2, float k) {
    float h = clamp(0.5 + 0.5 * (d2 - d1) / k, 0.0, 1.0);
    return mix(d2, d1, h) - k * h * (1.0 - h);
}

float opIntersection(float a, float b) {
    return max(a, b);
}

float opSubtraction(float a, float b) {
    return max(a, -b);
}

vec2 bisector(vec2 v0, vec2 v1, vec2 v2) {
    vec2 v01 = normalize(v1 - v0);
    vec2 v02 = normalize(v2 - v0);
    return normalize(v01 + v02);
}

float cotHalf(float cosTheta) {
    float sinTheta = sqrt(max(0.0, 1.0 - cosTheta * cosTheta));
    return (1.0 + cosTheta) / sinTheta;
}

void fitCircle(vec2 v0, vec2 v1, vec2 v2, float radius, inout float angle, inout float distance) {
    float cosTheta = dot(normalize(v1 - v0), normalize(v2 - v0));
    float sinHalf = sqrt((1.0 - cosTheta) * 0.5);

    angle = sinHalf;
    distance = radius / sinHalf;
}

float dRoundedTri(vec2 p, inout float dTri, vec2 v0, vec2 v1, vec2 v2, float radius) {
    // circle distance from vertex
    float angle;
    float distance;
    fitCircle(v0, v1, v2, radius, angle, distance);

    // circle used to round it
    vec2 bisect = bisector(v0, v1, v2);
    vec2 circlePos = v0 + bisect * distance;
    float dCircle = sdCircle(p - circlePos, radius);

    // plane to isolate the two halfs of the circle (make the circle cutout asymmetrical)
    float dPlaneIn = sdPlane(p - circlePos, -bisect, -radius * angle);

    // mask based on plane
    dTri = opIntersection(dTri, dPlaneIn);

    return dCircle;
}

float cross2(vec2 a, vec2 b) {
    return a.x * b.y - a.y * b.x;
}

float computeRadius(vec2 prevPoint, vec2 cornerPoint, vec2 nextPoint, float desiredCornerDistance) {
    float maxEdgeLenPercent = 0.5;
    float sinParallelTreshold = sin(5 * M_PI / 180);

    float edge1Len = distance(prevPoint, cornerPoint);
    float edge2Len = distance(nextPoint, cornerPoint);
    if (edge1Len == 0.0 || edge2Len == 0.0) {
        return 0;
    }

    // Compute the start and end points of the rounded corner
    vec2 edge1Vector = normalize(cornerPoint - prevPoint);
    vec2 edge2Vector = normalize(nextPoint - cornerPoint);

    float edge1MaxCornerDistance = edge1Len * maxEdgeLenPercent;
    float edge2MaxCornerDistance = edge2Len * maxEdgeLenPercent;
    float cornerDistance = min(desiredCornerDistance, min(edge1MaxCornerDistance, edge2MaxCornerDistance));

    vec2 startPoint = cornerPoint - edge1Vector * cornerDistance;
    vec2 endPoint = cornerPoint + edge2Vector * cornerDistance;

    // Perpendicular directions
    vec2 perp1Vector = vec2(-edge1Vector.y, edge1Vector.x);
    vec2 perp2Vector = vec2(-edge2Vector.y, edge2Vector.x);

    // Ensure perpendiculars point toward same side
    if (cross2(edge1Vector, edge2Vector) < 0) {
        perp1Vector *= -1.0;
        perp2Vector *= -1.0;
    }

    // Center = intersection of perpendiculars
    float perpCrossProduct = cross2(perp1Vector, perp2Vector);
    if (abs(perpCrossProduct) < sinParallelTreshold) {
        return 0;
    }
    float t = cross2(endPoint - startPoint, perp2Vector) / perpCrossProduct;
    vec2 centerPoint = startPoint + perp1Vector * t;

    float radius = distance(startPoint, centerPoint);
    return radius;
}
#endif
)";

template <>
struct ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillShader";

    static const std::array<AttributeInfo, 4> attributes;
    static const std::array<AttributeInfo, 1> instanceAttributes;
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = fillShaderPrelude;
    static constexpr auto vertex = R"(

#if !defined(HAS_UNIFORM_u_triangle_instance)
layout(location = 4) in uvec3 in_triangle_indices;
#endif

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct FillDrawableUBO {
    mat4 matrix;
    // Interpolations
    float color_t;
    float opacity_t;
    float pad1;
    float pad2;
    vec4 pad3;
    vec4 pad4;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillDrawableUBO) readonly buffer FillDrawableUBOVector {
    FillDrawableUBO drawable_ubo[];
} drawableVector;

struct PolygonVertex {
    int position;
};

layout(std430, set = DRAWABLE_UBO_SET_INDEX, binding = idFillPosVertexDrawableUBO) readonly buffer PolygonVertexDrawableUBOVector {
    PolygonVertex vertex[];
} polygonVertexVector;

#if !defined(HAS_UNIFORM_u_triangle_instance)
struct NeighborVertex {
    int prev_next_offset;
};

layout(std430, set = DRAWABLE_UBO_SET_INDEX, binding = idFillPrevNextVertexDrawableUBO) readonly buffer NeighborVertexDrawableUBOVector {
    NeighborVertex vertex[];
} neighborVertexVector;
#endif

#if !defined(HAS_UNIFORM_u_color) || !defined(HAS_UNIFORM_u_opacity) || !defined(HAS_UNIFORM_u_outline_color) || !defined(HAS_UNIFORM_u_pattern_to) || !defined(HAS_UNIFORM_u_pattern_from)
struct DataVertex {
#if !defined(HAS_UNIFORM_u_color)
    float color[4];
#endif
#if !defined(HAS_UNIFORM_u_outline_color)
    float outline_color[4];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float opacity[2];
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
    uint pattern_to[2];
#endif
#if !defined(HAS_UNIFORM_u_pattern_from)
    uint pattern_from[2];
#endif
};

layout(std430, set = DRAWABLE_UBO_SET_INDEX, binding = idFillDataVertexDrawableUBO) readonly buffer DataVertexDrawableUBOVector {
    DataVertex vertex[];
} dataVertexVector;
#endif

layout(set = LAYER_SET_INDEX, binding = idFillEvaluatedPropsUBO) uniform FillEvaluatedPropsUBO {
    vec4 color;
    vec4 outline_color;
    float opacity;
    float rounded_corner_distance;
    float fade;
    float from_scale;
    float to_scale;
    float pad1;
    float pad2;
    float pad3;
} props;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 0) out vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 1) out lowp float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_triangle_instance)
struct FragmentData {
    vec2 v0;
    vec2 v1;
    vec2 v2;

    vec2 v0Prev;
    vec2 v0Next;
    float v0Radius;

    vec2 v1Prev;
    vec2 v1Next;
    float v1Radius;

    vec2 v2Prev;
    vec2 v2Next;
    float v2Radius;

    float ext;
};
layout(location = 2) out vec2 frag_pos;
layout(location = 3) flat out FragmentData frag_data;
#endif

void main() {
    const FillDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

#if defined(HAS_UNIFORM_u_triangle_instance)
    uint index = gl_VertexIndex;
    vec2 position = unpack_int(polygonVertexVector.vertex[index].position);
#else
    uint index = in_triangle_indices[gl_VertexIndex] >> 2;
    vec2 position = unpack_int(polygonVertexVector.vertex[index].position);

    uint i0 = in_triangle_indices[0] >> 2;
    uint i1 = in_triangle_indices[1] >> 2;
    uint i2 = in_triangle_indices[2] >> 2;

    uint ignored0 = in_triangle_indices[0] % 4 >> 1;
    uint ignored1 = in_triangle_indices[1] % 4 >> 1;
    uint ignored2 = in_triangle_indices[2] % 4 >> 1;

    uint external0 = in_triangle_indices[0] % 2;
    uint external1 = in_triangle_indices[1] % 2;
    uint external2 = in_triangle_indices[2] % 2;

    vec2 v0 = unpack_int(polygonVertexVector.vertex[i0].position);
    vec2 v1 = unpack_int(polygonVertexVector.vertex[i1].position);
    vec2 v2 = unpack_int(polygonVertexVector.vertex[i2].position);

    vec2 v0Prev = v0;
    vec2 v0Next = v0;
    float v0Radius = 0;
    if (ignored0 == 0 && props.rounded_corner_distance > 0) {
        uint i0Prev = i0 + unpack_low_int(neighborVertexVector.vertex[i0].prev_next_offset);
        uint i0Next = i0 + unpack_high_int(neighborVertexVector.vertex[i0].prev_next_offset);

        v0Prev = unpack_int(polygonVertexVector.vertex[i0Prev].position);
        v0Next = unpack_int(polygonVertexVector.vertex[i0Next].position);
        v0Radius = computeRadius(v0Prev, v0, v0Next, props.rounded_corner_distance);
    }

    vec2 v1Prev = v1;
    vec2 v1Next = v1;
    float v1Radius = 0;
    if (ignored1 == 0 && props.rounded_corner_distance > 0) {
        uint i1Prev = i1 + unpack_low_int(neighborVertexVector.vertex[i1].prev_next_offset);
        uint i1Next = i1 + unpack_high_int(neighborVertexVector.vertex[i1].prev_next_offset);

        v1Prev = unpack_int(polygonVertexVector.vertex[i1Prev].position);
        v1Next = unpack_int(polygonVertexVector.vertex[i1Next].position);
        v1Radius = computeRadius(v1Prev, v1, v1Next, props.rounded_corner_distance);
    }

    vec2 v2Prev = v2;
    vec2 v2Next = v2;
    float v2Radius = 0;
    if (ignored2 == 0 && props.rounded_corner_distance > 0) {
        uint i2Prev = i2 + unpack_low_int(neighborVertexVector.vertex[i2].prev_next_offset);
        uint i2Next = i2 + unpack_high_int(neighborVertexVector.vertex[i2].prev_next_offset);

        v2Prev = unpack_int(polygonVertexVector.vertex[i2Prev].position);
        v2Next = unpack_int(polygonVertexVector.vertex[i2Next].position);
        v2Radius = computeRadius(v2Prev, v2, v2Next, props.rounded_corner_distance);
    }

    frag_pos = position;

    frag_data.v0 = v0;
    frag_data.v1 = v1;
    frag_data.v2 = v2;

    frag_data.v0Prev = v0Prev;
    frag_data.v0Next = v0Next;
    frag_data.v0Radius = v0Radius;

    frag_data.v1Prev = v1Prev;
    frag_data.v1Next = v1Next;
    frag_data.v1Radius = v1Radius;

    frag_data.v2Prev = v2Prev;
    frag_data.v2Next = v2Next;
    frag_data.v2Radius = v2Radius;

    frag_data.ext = (external0 > 0 || external1 > 0 || external2 > 0) ? -1.0 : 1.0;
#endif

#if !defined(HAS_UNIFORM_u_color)
    frag_color = vec4(unpack_mix_color(dataVertexVector.vertex[index].color, drawable.color_t));
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    frag_opacity = unpack_mix_float(dataVertexVector.vertex[index].opacity, drawable.opacity_t);
#endif

    gl_Position = drawable.matrix * vec4(position, 0.0, 1.0);
    applySurfaceTransform();
}
)";

    static constexpr auto fragment = R"(

#if !defined(HAS_UNIFORM_u_color)
layout(location = 0) in vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 1) in lowp float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_triangle_instance)
struct FragmentData {
    vec2 v0;
    vec2 v1;
    vec2 v2;

    vec2 v0Prev;
    vec2 v0Next;
    float v0Radius;

    vec2 v1Prev;
    vec2 v1Next;
    float v1Radius;

    vec2 v2Prev;
    vec2 v2Next;
    float v2Radius;

    float ext;
};
layout(location = 2) in vec2 frag_pos;
layout(location = 3) flat in FragmentData frag_data;
#endif

layout(location = 0) out vec4 out_color;

layout(set = LAYER_SET_INDEX, binding = idFillEvaluatedPropsUBO) uniform FillEvaluatedPropsUBO {
    vec4 color;
    vec4 outline_color;
    float opacity;
    float rounded_corner_distance;
    float fade;
    float from_scale;
    float to_scale;
    float pad1;
    float pad2;
    float pad3;
} props;

void main() {

#if defined(HAS_UNIFORM_u_color)
    const vec4 color = props.color;
#else
    const vec4 color = frag_color;
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const float opacity = props.opacity;
#else
    const float opacity = frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_triangle_instance)
    float dTri = -1;

    if (frag_data.v0Radius > 0) {
        float d0 = dRoundedTri(frag_pos, dTri, frag_data.v0, frag_data.v0Prev, frag_data.v0Next, frag_data.v0Radius);
        dTri = opUnion(dTri, d0);
    }

    if (frag_data.v1Radius > 0) {
        float d1 = dRoundedTri(frag_pos, dTri, frag_data.v1, frag_data.v1Prev, frag_data.v1Next, frag_data.v1Radius);
        dTri = opUnion(dTri, d1);
    }

    if (frag_data.v2Radius > 0) {
        float d2 = dRoundedTri(frag_pos, dTri, frag_data.v2, frag_data.v2Prev, frag_data.v2Next, frag_data.v2Radius);
        dTri = opUnion(dTri, d2);
    }

    if (dTri * frag_data.ext > 0) {
        discard;
    }
#endif

    out_color = color * opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillOutlineShader";

    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = fillShaderPrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;

#if !defined(HAS_UNIFORM_u_outline_color)
layout(location = 1) in vec4 in_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 2) in vec2 in_opacity;
#endif

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct FillOutlineDrawableUBO {
    mat4 matrix;
    // Interpolations
    float outline_color_t;
    float opacity_t;
    float pad1;
    float pad2;
    vec4 pad3;
    vec4 pad4;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillDrawableUBO) readonly buffer FillOutlineDrawableUBOVector {
    FillOutlineDrawableUBO drawable_ubo[];
} drawableVector;

#if !defined(HAS_UNIFORM_u_outline_color)
layout(location = 0) out vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 1) out lowp float frag_opacity;
#endif

layout(location = 2) out vec2 frag_position;

void main() {
    const FillOutlineDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

#if !defined(HAS_UNIFORM_u_outline_color)
    frag_color = vec4(unpack_mix_color(in_color, drawable.outline_color_t));
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    frag_opacity = unpack_mix_float(in_opacity, drawable.opacity_t);
#endif

    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    applySurfaceTransform();

    frag_position = (gl_Position.xy / gl_Position.w + 1.0) / 2.0 * paintParams.world_size;
}
)";

    static constexpr auto fragment = R"(

#if !defined(HAS_UNIFORM_u_outline_color)
layout(location = 0) in vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 1) in float frag_opacity;
#endif

layout(location = 2) in vec2 frag_position;

layout(location = 0) out vec4 out_color;

layout(set = LAYER_SET_INDEX, binding = idFillEvaluatedPropsUBO) uniform FillEvaluatedPropsUBO {
    vec4 color;
    vec4 outline_color;
    float opacity;
    float rounded_corner_distance;
    float fade;
    float from_scale;
    float to_scale;
    float pad1;
    float pad2;
    float pad3;
} props;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

#if defined(HAS_UNIFORM_u_outline_color)
    const vec4 color = props.outline_color;
#else
    const vec4 color = frag_color;
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const float opacity = props.opacity;
#else
    const float opacity = frag_opacity;
#endif

    float dist = length(frag_position - gl_FragCoord.xy);
    float alpha = 1.0 - smoothstep(0.0, 1.0, dist);
    out_color = color * opacity * alpha;
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillPatternShader";

    static const std::array<AttributeInfo, 6> attributes;
    static const std::array<AttributeInfo, 1> instanceAttributes;
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = fillShaderPrelude;
    static constexpr auto vertex = R"(

#if !defined(HAS_UNIFORM_u_triangle_instance)
layout(location = 6) in uvec3 in_triangle_indices;
#endif

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct FillPatternDrawableUBO {
    mat4 matrix;
    vec2 pixel_coord_upper;
    vec2 pixel_coord_lower;
    float tile_ratio;
    // Interpolations
    float pattern_from_t;
    float pattern_to_t;
    float opacity_t;
    float color_t;
    float pad1;
    float pad2;
    float pad3;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillDrawableUBO) readonly buffer FillPatternDrawableUBOVector {
    FillPatternDrawableUBO drawable_ubo[];
} drawableVector;

struct PolygonVertex {
    int position;
};

layout(std430, set = DRAWABLE_UBO_SET_INDEX, binding = idFillPosVertexDrawableUBO) readonly buffer PolygonVertexDrawableUBOVector {
    PolygonVertex vertex[];
} polygonVertexVector;

#if !defined(HAS_UNIFORM_u_triangle_instance)
struct NeighborVertex {
    int prev_next_offset;
};

layout(std430, set = DRAWABLE_UBO_SET_INDEX, binding = idFillPrevNextVertexDrawableUBO) readonly buffer NeighborVertexDrawableUBOVector {
    NeighborVertex vertex[];
} neighborVertexVector;
#endif

#if !defined(HAS_UNIFORM_u_color) || !defined(HAS_UNIFORM_u_opacity) || !defined(HAS_UNIFORM_u_outline_color) || !defined(HAS_UNIFORM_u_pattern_to) || !defined(HAS_UNIFORM_u_pattern_from)
struct DataVertex {
#if !defined(HAS_UNIFORM_u_color)
    float color[4];
#endif
#if !defined(HAS_UNIFORM_u_outline_color)
    float outline_color[4];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float opacity[2];
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
    uint pattern_to[2];
#endif
#if !defined(HAS_UNIFORM_u_pattern_from)
    uint pattern_from[2];
#endif
};

layout(std430, set = DRAWABLE_UBO_SET_INDEX, binding = idFillDataVertexDrawableUBO) readonly buffer DataVertexDrawableUBOVector {
    DataVertex vertex[];
} dataVertexVector;
#endif

struct FillPatternTilePropsUBO {
    vec4 pattern_from;
    vec4 pattern_to;
    vec2 texsize;
    float sdf;
    float pad2;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillTilePropsUBO) readonly buffer FillPatternTilePropsUBOVector {
    FillPatternTilePropsUBO tile_props_ubo[];
} tilePropsVector;

layout(set = LAYER_SET_INDEX, binding = idFillEvaluatedPropsUBO) uniform FillEvaluatedPropsUBO {
    vec4 color;
    vec4 outline_color;
    float opacity;
    float rounded_corner_distance;
    float fade;
    float from_scale;
    float to_scale;
    float pad1;
    float pad2;
    float pad3;
} props;

layout(location = 0) out vec2 frag_pos_a;
layout(location = 1) out vec2 frag_pos_b;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 2) out vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 3) out lowp float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 4) out mediump vec4 frag_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 5) out mediump vec4 frag_pattern_to;
#endif

#if !defined(HAS_UNIFORM_u_triangle_instance)
struct FragmentData {
    vec2 v0;
    vec2 v1;
    vec2 v2;

    vec2 v0Prev;
    vec2 v0Next;
    float v0Radius;

    vec2 v1Prev;
    vec2 v1Next;
    float v1Radius;

    vec2 v2Prev;
    vec2 v2Next;
    float v2Radius;

    float ext;
};
layout(location = 6) out vec2 frag_pos;
layout(location = 7) flat out FragmentData frag_data;
#endif

void main() {
    const FillPatternDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];
    const FillPatternTilePropsUBO tileProps = tilePropsVector.tile_props_ubo[constant.ubo_index];

#if defined(HAS_UNIFORM_u_triangle_instance)
    uint index = gl_VertexIndex;
    vec2 position = unpack_int(polygonVertexVector.vertex[index].position);
#else
    uint index = in_triangle_indices[gl_VertexIndex] >> 2;
    vec2 position = unpack_int(polygonVertexVector.vertex[index].position);

    uint i0 = in_triangle_indices[0] >> 2;
    uint i1 = in_triangle_indices[1] >> 2;
    uint i2 = in_triangle_indices[2] >> 2;

    uint ignored0 = in_triangle_indices[0] % 4 >> 1;
    uint ignored1 = in_triangle_indices[1] % 4 >> 1;
    uint ignored2 = in_triangle_indices[2] % 4 >> 1;

    uint external0 = in_triangle_indices[0] % 2;
    uint external1 = in_triangle_indices[1] % 2;
    uint external2 = in_triangle_indices[2] % 2;

    vec2 v0 = unpack_int(polygonVertexVector.vertex[i0].position);
    vec2 v1 = unpack_int(polygonVertexVector.vertex[i1].position);
    vec2 v2 = unpack_int(polygonVertexVector.vertex[i2].position);

    vec2 v0Prev = v0;
    vec2 v0Next = v0;
    float v0Radius = 0;
    if (ignored0 == 0 && props.rounded_corner_distance > 0) {
        uint i0Prev = i0 + unpack_low_int(neighborVertexVector.vertex[i0].prev_next_offset);
        uint i0Next = i0 + unpack_high_int(neighborVertexVector.vertex[i0].prev_next_offset);

        v0Prev = unpack_int(polygonVertexVector.vertex[i0Prev].position);
        v0Next = unpack_int(polygonVertexVector.vertex[i0Next].position);
        v0Radius = computeRadius(v0Prev, v0, v0Next, props.rounded_corner_distance);
    }

    vec2 v1Prev = v1;
    vec2 v1Next = v1;
    float v1Radius = 0;
    if (ignored1 == 0 && props.rounded_corner_distance > 0) {
        uint i1Prev = i1 + unpack_low_int(neighborVertexVector.vertex[i1].prev_next_offset);
        uint i1Next = i1 + unpack_high_int(neighborVertexVector.vertex[i1].prev_next_offset);

        v1Prev = unpack_int(polygonVertexVector.vertex[i1Prev].position);
        v1Next = unpack_int(polygonVertexVector.vertex[i1Next].position);
        v1Radius = computeRadius(v1Prev, v1, v1Next, props.rounded_corner_distance);
    }

    vec2 v2Prev = v2;
    vec2 v2Next = v2;
    float v2Radius = 0;
    if (ignored2 == 0 && props.rounded_corner_distance > 0) {
        uint i2Prev = i2 + unpack_low_int(neighborVertexVector.vertex[i2].prev_next_offset);
        uint i2Next = i2 + unpack_high_int(neighborVertexVector.vertex[i2].prev_next_offset);

        v2Prev = unpack_int(polygonVertexVector.vertex[i2Prev].position);
        v2Next = unpack_int(polygonVertexVector.vertex[i2Next].position);
        v2Radius = computeRadius(v2Prev, v2, v2Next, props.rounded_corner_distance);
    }

    frag_pos = position;

    frag_data.v0 = v0;
    frag_data.v1 = v1;
    frag_data.v2 = v2;

    frag_data.v0Prev = v0Prev;
    frag_data.v0Next = v0Next;
    frag_data.v0Radius = v0Radius;

    frag_data.v1Prev = v1Prev;
    frag_data.v1Next = v1Next;
    frag_data.v1Radius = v1Radius;

    frag_data.v2Prev = v2Prev;
    frag_data.v2Next = v2Next;
    frag_data.v2Radius = v2Radius;

    frag_data.ext = (external0 > 0 || external1 > 0 || external2 > 0) ? -1.0 : 1.0;
#endif
    
#if !defined(HAS_UNIFORM_u_color)
    frag_color = unpack_mix_color(dataVertexVector.vertex[index].color, drawable.color_t);
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    frag_opacity = unpack_mix_float(dataVertexVector.vertex[index].opacity, drawable.opacity_t);
#endif

#if defined(HAS_UNIFORM_u_pattern_from)
    const vec4 frag_pattern_from = tileProps.pattern_from;
#else
    frag_pattern_from = vec4(unpack_uint(dataVertexVector.vertex[index].pattern_from[0]),
                             unpack_uint(dataVertexVector.vertex[index].pattern_from[1]));
#endif

#if defined(HAS_UNIFORM_u_pattern_to)
    const vec4 frag_pattern_to = tileProps.pattern_to;
#else
    frag_pattern_to = vec4(unpack_uint(dataVertexVector.vertex[index].pattern_to[0]),
                           unpack_uint(dataVertexVector.vertex[index].pattern_to[1]));
#endif

    const vec2 pattern_tl_a = frag_pattern_from.xy;
    const vec2 pattern_br_a = frag_pattern_from.zw;
    const vec2 pattern_tl_b = frag_pattern_to.xy;
    const vec2 pattern_br_b = frag_pattern_to.zw;

    const float pixelRatio = paintParams.pixel_ratio;
    const float tileZoomRatio = drawable.tile_ratio;
    const float fromScale = props.from_scale;
    const float toScale = props.to_scale;

    const vec2 display_size_a = vec2((pattern_br_a.x - pattern_tl_a.x) / pixelRatio, (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    const vec2 display_size_b = vec2((pattern_br_b.x - pattern_tl_b.x) / pixelRatio, (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);

    frag_pos_a = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, fromScale * display_size_a, tileZoomRatio, position),
    frag_pos_b = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, toScale * display_size_b, tileZoomRatio, position),

    gl_Position = drawable.matrix * vec4(position, 0.0, 1.0);
    applySurfaceTransform();
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in vec2 frag_pos_a;
layout(location = 1) in vec2 frag_pos_b;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 2) in vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 3) in lowp float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 4) in vec4 frag_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 5) in vec4 frag_pattern_to;
#endif

#if !defined(HAS_UNIFORM_u_triangle_instance)
struct FragmentData {
    vec2 v0;
    vec2 v1;
    vec2 v2;

    vec2 v0Prev;
    vec2 v0Next;
    float v0Radius;

    vec2 v1Prev;
    vec2 v1Next;
    float v1Radius;

    vec2 v2Prev;
    vec2 v2Next;
    float v2Radius;

    float ext;
};
layout(location = 6) in vec2 frag_pos;
layout(location = 7) flat in FragmentData frag_data;
#endif

layout(location = 0) out vec4 out_color;

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct FillPatternTilePropsUBO {
    vec4 pattern_from;
    vec4 pattern_to;
    vec2 texsize;
    float sdf;
    float pad2;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillTilePropsUBO) readonly buffer FillPatternTilePropsUBOVector {
    FillPatternTilePropsUBO tile_props_ubo[];
} tilePropsVector;

layout(set = LAYER_SET_INDEX, binding = idFillEvaluatedPropsUBO) uniform FillEvaluatedPropsUBO {
    vec4 color;
    vec4 outline_color;
    float opacity;
    float rounded_corner_distance;
    float fade;
    float from_scale;
    float to_scale;
    float pad1;
    float pad2;
    float pad3;
} props;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image0_sampler;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    const FillPatternTilePropsUBO tileProps = tilePropsVector.tile_props_ubo[constant.ubo_index];

#if defined(HAS_UNIFORM_u_color)
    const vec4 color = props.color;
#else
    const vec4 color = frag_color;
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const float opacity = props.opacity;
#else
    const float opacity = frag_opacity;
#endif

#if defined(HAS_UNIFORM_u_pattern_from)
    const vec4 pattern_from = tileProps.pattern_from;
#else
    const vec4 pattern_from = frag_pattern_from;
#endif

#if defined(HAS_UNIFORM_u_pattern_to)
    const vec4 pattern_to = tileProps.pattern_to;
#else
    const vec4 pattern_to = frag_pattern_to;
#endif

#if !defined(HAS_UNIFORM_u_triangle_instance)
    float dTri = -1;

    if (frag_data.v0Radius > 0) {
        float d0 = dRoundedTri(frag_pos, dTri, frag_data.v0, frag_data.v0Prev, frag_data.v0Next, frag_data.v0Radius);
        dTri = opUnion(dTri, d0);
    }

    if (frag_data.v1Radius > 0) {
        float d1 = dRoundedTri(frag_pos, dTri, frag_data.v1, frag_data.v1Prev, frag_data.v1Next, frag_data.v1Radius);
        dTri = opUnion(dTri, d1);
    }

    if (frag_data.v2Radius > 0) {
        float d2 = dRoundedTri(frag_pos, dTri, frag_data.v2, frag_data.v2Prev, frag_data.v2Next, frag_data.v2Radius);
        dTri = opUnion(dTri, d2);
    }

    if (dTri * frag_data.ext > 0) {
        discard;
    }
#endif

    const vec2 pattern_tl_a = pattern_from.xy;
    const vec2 pattern_br_a = pattern_from.zw;
    const vec2 pattern_tl_b = pattern_to.xy;
    const vec2 pattern_br_b = pattern_to.zw;

    const vec2 imagecoord = mod(frag_pos_a, 1.0);
    const vec2 pos = mix(pattern_tl_a / tileProps.texsize, pattern_br_a / tileProps.texsize, imagecoord);
    const vec4 color1 = texture(image0_sampler, pos);

    const vec2 imagecoord_b = mod(frag_pos_b, 1.0);
    const vec2 pos2 = mix(pattern_tl_b / tileProps.texsize, pattern_br_b / tileProps.texsize, imagecoord_b);
    const vec4 color2 = texture(image0_sampler, pos2);

    if (tileProps.sdf > 0.5) {
        const float sdf_edge = (256.0 - 64.0) / 256.0;
        const float sdf_gamma_a = max(fwidth(color1.a) * 0.5, 1.0 / 255.0 / 16.0);
        const float sdf_gamma_b = max(fwidth(color2.a) * 0.5, 1.0 / 255.0 / 16.0);
        const float sdf_alpha_a = smoothstep(sdf_edge - sdf_gamma_a, sdf_edge + sdf_gamma_a, color1.a);
        const float sdf_alpha_b = smoothstep(sdf_edge - sdf_gamma_b, sdf_edge + sdf_gamma_b, color2.a);
        out_color = mix(color * sdf_alpha_a, color * sdf_alpha_b, props.fade) * opacity;
    } else {
        out_color = mix(color1, color2, props.fade) * opacity;
    }
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillOutlinePatternShader";

    static const std::array<AttributeInfo, 5> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = fillShaderPrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 1) in vec4 in_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 2) in vec2 in_opacity;
#endif

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 3) in mediump uvec4 in_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 4) in mediump uvec4 in_pattern_to;
#endif

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct FillOutlinePatternDrawableUBO {
    mat4 matrix;
    vec2 pixel_coord_upper;
    vec2 pixel_coord_lower;
    float tile_ratio;
    // Interpolations
    float pattern_from_t;
    float pattern_to_t;
    float opacity_t;
    float color_t;
    float pad1;
    float pad2;
    float pad3;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillDrawableUBO) readonly buffer FillOutlinePatternDrawableUBOVector {
    FillOutlinePatternDrawableUBO drawable_ubo[];
} drawableVector;

struct FillOutlinePatternTilePropsUBO {
    vec4 pattern_from;
    vec4 pattern_to;
    vec2 texsize;
    float sdf;
    float pad2;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillTilePropsUBO) readonly buffer FillOutlinePatternTilePropsUBOVector {
    FillOutlinePatternTilePropsUBO tile_props_ubo[];
} tilePropsVector;

layout(set = LAYER_SET_INDEX, binding = idFillEvaluatedPropsUBO) uniform FillEvaluatedPropsUBO {
    vec4 color;
    vec4 outline_color;
    float opacity;
    float rounded_corner_distance;
    float fade;
    float from_scale;
    float to_scale;
    float pad1;
    float pad2;
    float pad3;
} props;

layout(location = 0) out vec2 frag_pos_a;
layout(location = 1) out vec2 frag_pos_b;
layout(location = 2) out vec2 frag_pos;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 3) out vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 4) out lowp float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 5) out mediump vec4 frag_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 6) out mediump vec4 frag_pattern_to;
#endif

void main() {
    const FillOutlinePatternDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];
    const FillOutlinePatternTilePropsUBO tileProps = tilePropsVector.tile_props_ubo[constant.ubo_index];

#if !defined(HAS_UNIFORM_u_color)
    frag_color = unpack_mix_color(in_color, drawable.color_t);
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    frag_opacity = unpack_mix_float(in_opacity, drawable.opacity_t);
#endif

#if defined(HAS_UNIFORM_u_pattern_from)
    const vec4 frag_pattern_from = tileProps.pattern_from;
#else
    frag_pattern_from = in_pattern_from;
#endif

#if defined(HAS_UNIFORM_u_pattern_to)
    const vec4 frag_pattern_to = tileProps.pattern_to;
#else
    frag_pattern_to = in_pattern_to;
#endif

    const vec2 pattern_tl_a = frag_pattern_from.xy;
    const vec2 pattern_br_a = frag_pattern_from.zw;
    const vec2 pattern_tl_b = frag_pattern_to.xy;
    const vec2 pattern_br_b = frag_pattern_to.zw;

    const float pixelRatio = paintParams.pixel_ratio;
    const float tileZoomRatio = drawable.tile_ratio;
    const float fromScale = props.from_scale;
    const float toScale = props.to_scale;

    const vec2 display_size_a = vec2((pattern_br_a.x - pattern_tl_a.x) / pixelRatio, (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    const vec2 display_size_b = vec2((pattern_br_b.x - pattern_tl_b.x) / pixelRatio, (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);

    const vec2 position2 = in_position.xy;
    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    applySurfaceTransform();

    frag_pos_a = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, fromScale * display_size_a, tileZoomRatio, position2),
    frag_pos_b = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, toScale * display_size_b, tileZoomRatio, position2),
    frag_pos = (gl_Position.xy / gl_Position.w + 1.0) / 2.0 * paintParams.world_size;
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in vec2 frag_pos_a;
layout(location = 1) in vec2 frag_pos_b;
layout(location = 2) in vec2 frag_pos;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 3) in vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 4) in lowp float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 5) in vec4 frag_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 6) in vec4 frag_pattern_to;
#endif

layout(location = 0) out vec4 out_color;

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct FillOutlinePatternTilePropsUBO {
    vec4 pattern_from;
    vec4 pattern_to;
    vec2 texsize;
    float sdf;
    float pad2;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillTilePropsUBO) readonly buffer FillOutlinePatternTilePropsUBOVector {
    FillOutlinePatternTilePropsUBO tile_props_ubo[];
} tilePropsVector;

layout(set = LAYER_SET_INDEX, binding = idFillEvaluatedPropsUBO) uniform FillEvaluatedPropsUBO {
    vec4 color;
    vec4 outline_color;
    float opacity;
    float rounded_corner_distance;
    float fade;
    float from_scale;
    float to_scale;
    float pad1;
    float pad2;
    float pad3;
} props;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image0_sampler;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    const FillOutlinePatternTilePropsUBO tileProps = tilePropsVector.tile_props_ubo[constant.ubo_index];

#if defined(HAS_UNIFORM_u_color)
    const vec4 color = props.color;
#else
    const vec4 color = frag_color;
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const float opacity = props.opacity;
#else
    const float opacity = frag_opacity;
#endif

#if defined(HAS_UNIFORM_u_pattern_from)
    const vec4 pattern_from = tileProps.pattern_from;
#else
    const vec4 pattern_from = frag_pattern_from;
#endif

#if defined(HAS_UNIFORM_u_pattern_to)
    const vec4 pattern_to = tileProps.pattern_to;
#else
    const vec4 pattern_to = frag_pattern_to;
#endif

    const vec2 pattern_tl_a = pattern_from.xy;
    const vec2 pattern_br_a = pattern_from.zw;
    const vec2 pattern_tl_b = pattern_to.xy;
    const vec2 pattern_br_b = pattern_to.zw;

    const vec2 imagecoord = mod(frag_pos_a, 1.0);
    const vec2 pos = mix(pattern_tl_a / tileProps.texsize, pattern_br_a / tileProps.texsize, imagecoord);
    const vec4 color1 = texture(image0_sampler, pos);

    const vec2 imagecoord_b = mod(frag_pos_b, 1.0);
    const vec2 pos2 = mix(pattern_tl_b / tileProps.texsize, pattern_br_b / tileProps.texsize, imagecoord_b);
    const vec4 color2 = texture(image0_sampler, pos2);

    const float dist = length(frag_pos - gl_FragCoord.xy);
    const float alpha = 1.0 - smoothstep(0.0, 1.0, dist);

    if (tileProps.sdf > 0.5) {
        const float sdf_edge = (256.0 - 64.0) / 256.0;
        const float sdf_gamma_a = max(fwidth(color1.a) * 0.5, 1.0 / 255.0 / 16.0);
        const float sdf_gamma_b = max(fwidth(color2.a) * 0.5, 1.0 / 255.0 / 16.0);
        const float sdf_alpha_a = smoothstep(sdf_edge - sdf_gamma_a, sdf_edge + sdf_gamma_a, color1.a);
        const float sdf_alpha_b = smoothstep(sdf_edge - sdf_gamma_b, sdf_edge + sdf_gamma_b, color2.a);
        out_color = mix(color * sdf_alpha_a, color * sdf_alpha_b, props.fade) * alpha * opacity;
    } else {
        out_color = mix(color1, color2, props.fade) * alpha * opacity;
    }
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillOutlineTriangulatedShader";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = fillShaderPrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_pos_normal;
layout(location = 1) in uvec4 in_data;

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct FillOutlineTriangulatedDrawableUBO {
    mat4 matrix;
    float ratio;
    float pad1;
    float pad2;
    float pad3;
    vec4 pad4;
    vec4 pad5;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillDrawableUBO) readonly buffer FillOutlineTriangulatedDrawableUBOVector {
    FillOutlineTriangulatedDrawableUBO drawable_ubo[];
} drawableVector;

layout(location = 0) out float frag_width2;
layout(location = 1) out vec2 frag_normal;
layout(location = 2) out float frag_gamma_scale;

void main() {
    const FillOutlineTriangulatedDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;

    const vec2 a_extrude = in_data.xy - 128.0;
    const vec2 pos = floor(in_pos_normal * 0.5);

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of in_pos_normal
    mediump vec2 normal = in_pos_normal - 2.0 * pos;
    frag_normal = vec2(normal.x, normal.y * 2.0 - 1.0);
    frag_normal.y *= -1.0;

    // these transformations used to be applied in the JS and native code bases.
    // moved them into the shader for clarity and simplicity.
    float width = 1.0;
    float halfwidth = width / 2.0;
    float outset = halfwidth + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);

    // Scale the extrusion vector down to a normal and then up by the line width
    // of this vertex.
    mediump vec2 dist = outset * a_extrude * LINE_NORMAL_SCALE;

    vec4 projected_extrude = drawable.matrix * vec4(dist / drawable.ratio, 0.0, 0.0);
    gl_Position = drawable.matrix * vec4(pos, 0.0, 1.0) + projected_extrude;
    applySurfaceTransform();

    // calculate how much the perspective view squishes or stretches the extrude
    float extrude_length_without_perspective = length(dist);
    float extrude_length_with_perspective = length(projected_extrude.xy / gl_Position.w * paintParams.units_to_pixels);

    frag_width2 = outset;
    frag_gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective;
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in float frag_width2;
layout(location = 1) in vec2 frag_normal;
layout(location = 2) in float frag_gamma_scale;

layout(location = 0) out vec4 out_color;

layout(set = LAYER_SET_INDEX, binding = idFillEvaluatedPropsUBO) uniform FillEvaluatedPropsUBO {
    vec4 color;
    vec4 outline_color;
    float opacity;
    float fade;
    float from_scale;
    float to_scale;
} props;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    // Calculate the distance of the pixel from the line in pixels.
    const float dist = length(frag_normal) * frag_width2;

    // Calculate the antialiasing fade factor. This is either when fading in the
    // line in case of an offset line (`v_width2.y`) or when fading out (`v_width2.x`)
    const float blur2 = (1.0 / DEVICE_PIXEL_RATIO) * frag_gamma_scale;
    const float alpha = clamp(min(dist + blur2, frag_width2 - dist) / blur2, 0.0, 1.0);

    out_color = props.outline_color * alpha * props.opacity;
}
)";
};

} // namespace shaders
} // namespace mln
