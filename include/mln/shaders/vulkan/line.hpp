#pragma once

#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/vulkan/shader_program.hpp>

namespace mln {
namespace shaders {

constexpr auto lineShadePrelude = R"(

#define idLineDrawableUBO               idDrawableReservedVertexOnlyUBO
#define idLineTilePropsUBO              idDrawableReservedFragmentOnlyUBO
#define idLineEvaluatedPropsUBO         layerUBOStartId
#define idLineDataVertexDrawableUBO     drawableSSBOStartId

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
    float maxEdgeLenPercent = 1.0;
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
)";

template <>
struct ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "LineShader";

    static const std::array<AttributeInfo, 8> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = lineShadePrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_pos_normal;
layout(location = 1) in uvec4 in_data;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 2) in vec4 in_color;
#endif

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 3) in vec2 in_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 4) in vec2 in_opacity;
#endif

#if !defined(HAS_UNIFORM_u_gapwidth)
layout(location = 5) in vec2 in_gapwidth;
#endif

#if !defined(HAS_UNIFORM_u_offset)
layout(location = 6) in vec2 in_offset;
#endif

#if !defined(HAS_UNIFORM_u_width)
layout(location = 7) in vec2 in_width;
#endif

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct LineDrawableUBO {
    mat4 matrix;
    mediump float ratio;
    // Interpolations
    float color_t;
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float pad1;
    vec4 pad2;
    vec4 pad3;
};

layout(std140, set = LAYER_SET_INDEX, binding = idLineDrawableUBO) readonly buffer LineDrawableUBOVector {
    LineDrawableUBO drawable_ubo[];
} drawableVector;

layout(set = LAYER_SET_INDEX, binding = idLineEvaluatedPropsUBO) uniform LineEvaluatedPropsUBO {
    vec4 color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;
    uint expressionMask;
    float intersection_rounded_corner_distance;
} props;

layout(location = 0) out lowp vec2 frag_normal;
layout(location = 1) out lowp vec2 frag_width2;
layout(location = 2) out lowp float frag_gamma_scale;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 3) out lowp vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 4) out lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 5) out lowp float frag_opacity;
#endif

void main() {
    const LineDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

#ifndef HAS_UNIFORM_u_color
    frag_color = unpack_mix_color(in_color, drawable.color_t);
#endif

#ifndef HAS_UNIFORM_u_blur
    frag_blur = unpack_mix_float(in_blur, drawable.blur_t);
#endif

#ifndef HAS_UNIFORM_u_opacity
    frag_opacity = unpack_mix_float(in_opacity, drawable.opacity_t);
#endif

#ifndef HAS_UNIFORM_u_gapwidth
    const mediump float gapwidth = unpack_mix_float(in_gapwidth, drawable.gapwidth_t) / 2.0;
#else
    const mediump float gapwidth = props.gapwidth / 2.0;
#endif

#ifndef HAS_UNIFORM_u_offset
    const lowp float offset = unpack_mix_float(in_offset, drawable.offset_t) * -1.0;
#else
    const lowp float offset = props.offset * -1.0;
#endif

#ifndef HAS_UNIFORM_u_width
    mediump float width = unpack_mix_float(in_width, drawable.width_t);
#else
    mediump float width = props.width;
#endif

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;

    vec2 a_extrude = in_data.xy - 128.0;
    float a_direction = mod(in_data.z, 4.0) - 1.0;
    vec2 pos = floor(in_pos_normal * 0.5);

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of in_pos_normal
    mediump vec2 normal = in_pos_normal - 2.0 * pos;
    frag_normal = vec2(normal.x, normal.y * 2.0 - 1.0);

    // these transformations used to be applied in the JS and native code bases.
    // moved them into the shader for clarity and simplicity.
    float halfwidth = width / 2.0;

    float inset = gapwidth + (gapwidth > 0.0 ? ANTIALIASING : 0.0);
    float outset = gapwidth + halfwidth * (gapwidth > 0.0 ? 2.0 : 1.0) + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);

    // Scale the extrusion vector down to a normal and then up by the line width
    // of this vertex.
    mediump vec2 dist = outset * a_extrude * LINE_NORMAL_SCALE;

    // Calculate the offset when drawing a line that is to the side of the actual line.
    // We do this by creating a vector that points towards the extrude, but rotate
    // it when we're drawing round end points (a_direction = -1 or 1) since their
    // extrude vector points in another direction.
    mediump float u = 0.5 * a_direction;
    mediump float t = 1.0 - abs(u);
    mediump vec2 offset2 = offset * a_extrude * LINE_NORMAL_SCALE * frag_normal.y * mat2(t, -u, u, t);

    vec4 projected_extrude = drawable.matrix * vec4(dist / drawable.ratio, 0.0, 0.0);
    gl_Position = drawable.matrix * vec4(pos + offset2 / drawable.ratio, 0.0, 1.0) + projected_extrude;
    applySurfaceTransform();

    // calculate how much the perspective view squishes or stretches the extrude
    float extrude_length_without_perspective = length(dist);
    float extrude_length_with_perspective = length(projected_extrude.xy / gl_Position.w * paintParams.units_to_pixels);
    frag_gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective;

    frag_width2 = vec2(outset, inset);
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in lowp vec2 frag_normal;
layout(location = 1) in lowp vec2 frag_width2;
layout(location = 2) in lowp float frag_gamma_scale;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 3) in lowp vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 4) in lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 5) in lowp float frag_opacity;
#endif

layout(location = 0) out vec4 out_color;

layout(set = LAYER_SET_INDEX, binding = idLineEvaluatedPropsUBO) uniform LineEvaluatedPropsUBO {
    vec4 color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;
    uint expressionMask;
    float intersection_rounded_corner_distance;
} props;

void main() {

#ifdef OVERDRAW_INSPECTOR
    out_color = vec4(1.0);
    return;
#endif

#ifdef HAS_UNIFORM_u_color
    highp vec4 color = props.color;
#else
    highp vec4 color = frag_color;
#endif

#ifdef HAS_UNIFORM_u_blur
    lowp float blur = props.blur;
#else
    lowp float blur = frag_blur;
#endif

#ifdef HAS_UNIFORM_u_opacity
    lowp float opacity = props.opacity;
#else
    lowp float opacity = frag_opacity;
#endif

    // Calculate the distance of the pixel from the line in pixels.
    float dist = length(frag_normal) * frag_width2.s;

    // Calculate the antialiasing fade factor. This is either when fading in
    // the line in case of an offset line (frag_width2.t) or when fading out
    // (frag_width2.s)
    float blur2 = (blur + 1.0 / DEVICE_PIXEL_RATIO) * frag_gamma_scale;
    float alpha = clamp(min(dist - (frag_width2.t - blur2), frag_width2.s - dist) / blur2, 0.0, 1.0);

    out_color = color * (alpha * opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::LineIntersectionShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "LineIntersectionShader";

    static const std::array<AttributeInfo, 7> attributes;
    static const std::array<AttributeInfo, 2> instanceAttributes;
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = lineShadePrelude;
    static constexpr auto vertex = R"(

layout(location = 7) in uvec3 in_triangle_vertices;
layout(location = 8) in uvec2 in_data_indices;

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct LineDrawableUBO {
    mat4 matrix;
    mediump float ratio;
    // Interpolations
    float color_t;
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float pad1;
    vec4 pad2;
    vec4 pad3;
};

layout(std140, set = LAYER_SET_INDEX, binding = idLineDrawableUBO) readonly buffer LineDrawableUBOVector {
    LineDrawableUBO drawable_ubo[];
} drawableVector;

#if !defined(HAS_UNIFORM_u_color) || !defined(HAS_UNIFORM_u_blur) || !defined(HAS_UNIFORM_u_opacity) || !defined(HAS_UNIFORM_u_gapwidth) || !defined(HAS_UNIFORM_u_offset) || !defined(HAS_UNIFORM_u_width)
// Same order as in LinePaintProperties
struct DataVertex {
#if !defined(HAS_UNIFORM_u_color)
    float color[4];
#endif
#if !defined(HAS_UNIFORM_u_blur)
    float blur[2];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float opacity[2];
#endif
#if !defined(HAS_UNIFORM_u_gapwidth)
    float gapwidth[2];
#endif
#if !defined(HAS_UNIFORM_u_offset)
    float offset[2];
#endif
#if !defined(HAS_UNIFORM_u_width)
    float width[2];
#endif
};

layout(std430, set = DRAWABLE_UBO_SET_INDEX, binding = idLineDataVertexDrawableUBO) readonly buffer DataVertexDrawableUBOVector {
    DataVertex vertex[];
} dataVertexVector;
#endif

layout(set = LAYER_SET_INDEX, binding = idLineEvaluatedPropsUBO) uniform LineEvaluatedPropsUBO {
    vec4 color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;
    uint expressionMask;
    float intersection_rounded_corner_distance;
} props;

struct FragmentData {
    vec2 triangle_corner_prev;
    vec2 triangle_corner_pos;
    vec2 triangle_corner_next;
    float triangle_corner_radius;

#if !defined(HAS_UNIFORM_u_color)
    lowp vec4 color1;
    lowp vec4 color2;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    lowp float opacity1;
    lowp float opacity2;
#endif
};
layout(location = 0) out vec2 frag_triangle_pos;
layout(location = 1) flat out FragmentData frag_data;

void main() {
    const LineDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

    frag_triangle_pos = vec2(0.0);

    frag_data.triangle_corner_prev = vec2(0.0);
    frag_data.triangle_corner_pos = vec2(0.0);
    frag_data.triangle_corner_next = vec2(0.0);
    frag_data.triangle_corner_radius = 0.0;

#ifndef HAS_UNIFORM_u_color
    frag_data.color1 = vec4(0.0);
    frag_data.color2 = vec4(0.0);
#endif
#ifndef HAS_UNIFORM_u_opacity
    frag_data.opacity1 = 0.0;
    frag_data.opacity2 = 0.0;
#endif
    gl_Position = vec4(0.0);

    if (props.intersection_rounded_corner_distance <= 0.0) {
        return;
    }

    vec2 initial_vertex_pos[3];
    vec2 triangle_vertex_pos[3];
    float triangle_corner_radius;

    for (int i = 0; i < 3; i++) {
        int pos_x = int(in_triangle_vertices[i]) >> 16;
        int pos_y = (int(in_triangle_vertices[i]) << 16) >> 16;

        initial_vertex_pos[i] = vec2(pos_x, pos_y);
        triangle_vertex_pos[i] = initial_vertex_pos[i];
    }

    vec2 edgePrev = normalize(triangle_vertex_pos[0] - triangle_vertex_pos[1]);
    vec2 edgeNext = normalize(triangle_vertex_pos[2] - triangle_vertex_pos[1]);
    float sinAngle = cross2(edgeNext, edgePrev);

    vec2 perpEdgePrev = vec2(-edgePrev.y, edgePrev.x);
    vec2 perpEdgeNext = vec2(-edgeNext.y, edgeNext.x);

    for (int i = 0; i < 2; i++) {
        uint dataIndex = in_data_indices[i] / 2u;
        float direction = (float(in_data_indices[i] % 2u) * 2.0 - 1.0);

#ifndef HAS_UNIFORM_u_gapwidth
        const mediump float gapwidth = unpack_mix_float(dataVertexVector.vertex[dataIndex].gapwidth, drawable.gapwidth_t) / 2.0;
#else
        const mediump float gapwidth = props.gapwidth / 2.0;
#endif

#ifndef HAS_UNIFORM_u_offset
        const lowp float offset = unpack_mix_float(dataVertexVector.vertex[dataIndex].offset, drawable.offset_t);
#else
        const lowp float offset = props.offset;
#endif

#ifndef HAS_UNIFORM_u_width
        mediump float width = unpack_mix_float(dataVertexVector.vertex[dataIndex].width, drawable.width_t);
#else
        mediump float width = props.width;
#endif

#ifndef HAS_UNIFORM_u_blur
        lowp float blur = unpack_mix_float(dataVertexVector.vertex[dataIndex].blur, drawable.blur_t);
#else
        lowp float blur = props.blur;
#endif

        const float halfwidth = width / 2.0;
        const float outset = max(gapwidth + halfwidth * (gapwidth > 0.0 ? 2.0 : 1.0) - blur, 0.0);

        vec2 outsetVector = (i == 0 ? edgeNext : edgePrev);
        vec2 offsetVector = (i == 0 ? -edgeNext : edgePrev);
        const vec2 distCorner = (outset * outsetVector + offset * direction * offsetVector) / sinAngle;

        triangle_vertex_pos[1] = triangle_vertex_pos[1] + distCorner / drawable.ratio;

        vec2 outsetPerpVector = (i == 0 ? -perpEdgePrev : perpEdgeNext);
        vec2 offsetPerpVector = (i == 0 ? perpEdgePrev : perpEdgeNext);
        const vec2 distPerp = outset * outsetPerpVector + offset * direction * offsetPerpVector;

        triangle_vertex_pos[i * 2] = triangle_vertex_pos[i * 2] + distPerp / drawable.ratio;
    }

    for (int i = 0; i < 3; i+=2) {
        vec2 perpEdgeVector = triangle_vertex_pos[i] - initial_vertex_pos[i];
        vec2 initialCornerVector = initial_vertex_pos[1] - initial_vertex_pos[i];
        vec2 finalCornerVector = triangle_vertex_pos[1] - initial_vertex_pos[i];

        float initialCornerSide = cross2(perpEdgeVector, initialCornerVector);
        float finalCornerSide = cross2(perpEdgeVector, finalCornerVector);

        if (initialCornerSide * finalCornerSide <= 0) {
            return;
        }
    }

    triangle_corner_radius = computeRadius(triangle_vertex_pos[0],
                                           triangle_vertex_pos[1],
                                           triangle_vertex_pos[2],
                                           props.intersection_rounded_corner_distance);

    vec2 position = triangle_vertex_pos[gl_VertexIndex];
    uint dataIndex1 = in_data_indices[0] / 2;
    uint dataIndex2 = in_data_indices[1] / 2;

    frag_triangle_pos = position;

    frag_data.triangle_corner_prev = triangle_vertex_pos[0];
    frag_data.triangle_corner_pos = triangle_vertex_pos[1];
    frag_data.triangle_corner_next = triangle_vertex_pos[2];
    frag_data.triangle_corner_radius = triangle_corner_radius;

#ifndef HAS_UNIFORM_u_color
    frag_data.color1 = unpack_mix_float(dataVertexVector.vertex[dataIndex1].color, drawable.color_t);
    frag_data.color2 = unpack_mix_float(dataVertexVector.vertex[dataIndex2].color, drawable.color_t);;
#endif
#ifndef HAS_UNIFORM_u_opacity
    frag_data.opacity1 = unpack_mix_float(dataVertexVector.vertex[dataIndex1].opacity, drawable.opacity_t);
    frag_data.opacity2 = unpack_mix_float(dataVertexVector.vertex[dataIndex2].opacity, drawable.opacity_t);
#endif
    gl_Position = drawable.matrix * vec4(position, 0.0, 1.0);
    applySurfaceTransform();
}
)";

    static constexpr auto fragment = R"(

struct FragmentData {
    vec2 triangle_corner_prev;
    vec2 triangle_corner_pos;
    vec2 triangle_corner_next;
    float triangle_corner_radius;

#if !defined(HAS_UNIFORM_u_color)
    lowp vec4 color1;
    lowp vec4 color2;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    lowp float opacity1;
    lowp float opacity2;
#endif
};
layout(location = 0) in vec2 frag_triangle_pos;
layout(location = 1) flat in FragmentData frag_data;

layout(location = 0) out vec4 out_color;

layout(set = LAYER_SET_INDEX, binding = idLineEvaluatedPropsUBO) uniform LineEvaluatedPropsUBO {
    vec4 color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;
    uint expressionMask;
    float intersection_rounded_corner_distance;
} props;

void main() {

#ifdef OVERDRAW_INSPECTOR
    out_color = vec4(1.0);
    return;
#endif

    out_color = vec4(0.0);

    if (frag_data.triangle_corner_radius <= 0.0) {
        return;
    }

#if !defined(HAS_UNIFORM_u_color) || !defined(HAS_UNIFORM_u_opacity)
    vec2 vPos = frag_triangle_pos - frag_data.triangle_corner_pos;
    vec2 vPrev = frag_data.triangle_corner_prev - frag_data.triangle_corner_pos;
    vec2 vNext = frag_data.triangle_corner_next - frag_data.triangle_corner_pos;

    float angle = atan2(cross2(vPos, vPrev), dot(vPos, vPrev));
    float angleMax = atan2(cross2(vNext, vPrev), dot(vNext, vPrev));
#endif

#ifdef HAS_UNIFORM_u_color
    highp vec4 color = props.color;
#else
    highp vec4 color = mix(frag_data.color1, frag_data.color2, angle / angleMax);
#endif

#ifdef HAS_UNIFORM_u_opacity
    lowp float opacity = props.opacity;
#else
    lowp float opacity = mix(frag_data.opacity1, frag_data.opacity2, angle / angleMax);
#endif

    float dTri = -1.0;

    float d0 = dRoundedTri(frag_triangle_pos, dTri,
                           frag_data.triangle_corner_pos,
                           frag_data.triangle_corner_prev,
                           frag_data.triangle_corner_next,
                           frag_data.triangle_corner_radius);
    dTri = opUnion(dTri, d0);

    if (dTri < 0.0) {
        return;
    }

    out_color = color * opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "LineGradientShader";

    static const std::array<AttributeInfo, 7> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = lineShadePrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_pos_normal;
layout(location = 1) in uvec4 in_data;

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 2) in vec2 in_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 3) in vec2 in_opacity;
#endif

#if !defined(HAS_UNIFORM_u_gapwidth)
layout(location = 4) in vec2 in_gapwidth;
#endif

#if !defined(HAS_UNIFORM_u_offset)
layout(location = 5) in vec2 in_offset;
#endif

#if !defined(HAS_UNIFORM_u_width)
layout(location = 6) in vec2 in_width;
#endif

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct LineGradientDrawableUBO {
    mat4 matrix;
    mediump float ratio;
    // Interpolations
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float pad1;
    float pad2;
    vec4 pad3;
    vec4 pad4;
};

layout(std140, set = LAYER_SET_INDEX, binding = idLineDrawableUBO) readonly buffer LineGradientDrawableUBOVector {
    LineGradientDrawableUBO drawable_ubo[];
} drawableVector;

layout(set = LAYER_SET_INDEX, binding = idLineEvaluatedPropsUBO) uniform LineEvaluatedPropsUBO {
    vec4 color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;
    uint expressionMask;
    float intersection_rounded_corner_distance;
} props;

layout(location = 0) out lowp vec2 frag_normal;
layout(location = 1) out lowp vec2 frag_width2;
layout(location = 2) out lowp float frag_gamma_scale;
layout(location = 3) out float frag_lineprogress;

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 4) out lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 5) out lowp float frag_opacity;
#endif

void main() {
    const LineGradientDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

#ifndef HAS_UNIFORM_u_blur
    frag_blur = unpack_mix_float(in_blur, drawable.blur_t);
#endif

#ifndef HAS_UNIFORM_u_opacity
    frag_opacity = unpack_mix_float(in_opacity, drawable.opacity_t);
#endif

#ifndef HAS_UNIFORM_u_gapwidth
    mediump float gapwidth = unpack_mix_float(in_gapwidth, drawable.gapwidth_t) / 2.0;
#else
    mediump float gapwidth = props.gapwidth / 2.0;
#endif

#ifndef HAS_UNIFORM_u_offset
    const lowp float offset = unpack_mix_float(in_offset, drawable.offset_t) * -1.0;
#else
    const lowp float offset = props.offset * -1.0;
#endif

#ifndef HAS_UNIFORM_u_width
    mediump float width = unpack_mix_float(in_width, drawable.width_t);
#else
    mediump float width = props.width;
#endif

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;

    vec2 a_extrude = in_data.xy - 128.0;
    float a_direction = mod(in_data.z, 4.0) - 1.0;
    vec2 pos = floor(in_pos_normal * 0.5);
    frag_lineprogress = (floor(in_data.z / 4.0) + in_data.w * 64.0) * 2.0 / MAX_LINE_DISTANCE;

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of in_pos_normal
    mediump vec2 normal = in_pos_normal - 2.0 * pos;
    frag_normal = vec2(normal.x, normal.y * 2.0 - 1.0);

    // these transformations used to be applied in the JS and native code bases.
    // moved them into the shader for clarity and simplicity.
    float halfwidth = width / 2.0;

    float inset = gapwidth + (gapwidth > 0.0 ? ANTIALIASING : 0.0);
    float outset = gapwidth + halfwidth * (gapwidth > 0.0 ? 2.0 : 1.0) + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);

    // Scale the extrusion vector down to a normal and then up by the line width
    // of this vertex.
    mediump vec2 dist = outset * a_extrude * LINE_NORMAL_SCALE;

    // Calculate the offset when drawing a line that is to the side of the actual line.
    // We do this by creating a vector that points towards the extrude, but rotate
    // it when we're drawing round end points (a_direction = -1 or 1) since their
    // extrude vector points in another direction.
    mediump float u = 0.5 * a_direction;
    mediump float t = 1.0 - abs(u);
    mediump vec2 offset2 = offset * a_extrude * LINE_NORMAL_SCALE * frag_normal.y * mat2(t, -u, u, t);

    vec4 projected_extrude = drawable.matrix * vec4(dist / drawable.ratio, 0.0, 0.0);
    gl_Position = drawable.matrix * vec4(pos + offset2 / drawable.ratio, 0.0, 1.0) + projected_extrude;
    applySurfaceTransform();

    // calculate how much the perspective view squishes or stretches the extrude
    float extrude_length_without_perspective = length(dist);
    float extrude_length_with_perspective = length(projected_extrude.xy / gl_Position.w * paintParams.units_to_pixels);
    frag_gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective;

    frag_width2 = vec2(outset, inset);
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in lowp vec2 frag_normal;
layout(location = 1) in lowp vec2 frag_width2;
layout(location = 2) in lowp float frag_gamma_scale;
layout(location = 3) in float frag_lineprogress;

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 4) in lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 5) in lowp float frag_opacity;
#endif

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image0_sampler;

layout(location = 0) out vec4 out_color;

layout(set = LAYER_SET_INDEX, binding = idLineEvaluatedPropsUBO) uniform LineEvaluatedPropsUBO {
    vec4 color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;
    uint expressionMask;
    float intersection_rounded_corner_distance;
} props;

void main() {

#ifdef OVERDRAW_INSPECTOR
    out_color = vec4(1.0);
    return;
#endif

#ifdef HAS_UNIFORM_u_blur
    lowp float blur = props.blur;
#else
    lowp float blur = frag_blur;
#endif

#ifdef HAS_UNIFORM_u_opacity
    lowp float opacity = props.opacity;
#else
    lowp float opacity = frag_opacity;
#endif

    // Calculate the distance of the pixel from the line in pixels.
    float dist = length(frag_normal) * frag_width2.s;

    // Calculate the antialiasing fade factor. This is either when fading in
    // the line in case of an offset line (frag_width2.t) or when fading out
    // (frag_width2.s)
    float blur2 = (blur + 1.0 / DEVICE_PIXEL_RATIO) * frag_gamma_scale;
    float alpha = clamp(min(dist - (frag_width2.t - blur2), frag_width2.s - dist) / blur2, 0.0, 1.0);

    // For gradient lines, lineprogress is the ratio along the entire line,
    // scaled to [0, 2^15), and the gradient ramp is stored in a texture.
    const vec4 color = texture(image0_sampler, vec2(frag_lineprogress, 0.5));

    out_color = color * (alpha * opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "LinePatternShader";

    static const std::array<AttributeInfo, 9> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = lineShadePrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_pos_normal;
layout(location = 1) in uvec4 in_data;

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 2) in vec2 in_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 3) in vec2 in_opacity;
#endif

#if !defined(HAS_UNIFORM_u_gapwidth)
layout(location = 4) in vec2 in_gapwidth;
#endif

#if !defined(HAS_UNIFORM_u_offset)
layout(location = 5) in vec2 in_offset;
#endif

#if !defined(HAS_UNIFORM_u_width)
layout(location = 6) in vec2 in_width;
#endif

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 7) in uvec4 in_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 8) in uvec4 in_pattern_to;
#endif

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct LinePatternDrawableUBO {
    mat4 matrix;
    float ratio;
    // Interpolations
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float pattern_from_t;
    float pattern_to_t;
    float floorwidth_t;
    float pad1;
    float pad2;
    float pad3;
    vec4 pad4;
};

layout(std140, set = LAYER_SET_INDEX, binding = idLineDrawableUBO) readonly buffer LinePatternDrawableUBOVector {
    LinePatternDrawableUBO drawable_ubo[];
} drawableVector;

layout(set = LAYER_SET_INDEX, binding = idLineEvaluatedPropsUBO) uniform LineEvaluatedPropsUBO {
    vec4 color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;
    uint expressionMask;
    float intersection_rounded_corner_distance;
} props;

layout(location = 0) out lowp vec2 frag_normal;
layout(location = 1) out lowp vec2 frag_width2;
layout(location = 2) out lowp float frag_gamma_scale;
layout(location = 3) out float frag_linesofar;

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 4) out lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 5) out lowp float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 6) out mediump vec4 frag_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 7) out mediump vec4 frag_pattern_to;
#endif
layout(location = 8) out mediump float frag_floorwidth;

void main() {
    const LinePatternDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

#ifndef HAS_UNIFORM_u_blur
    frag_blur = unpack_mix_float(in_blur, drawable.blur_t);
#endif

#ifndef HAS_UNIFORM_u_opacity
    frag_opacity = unpack_mix_float(in_opacity, drawable.opacity_t);
#endif

#ifndef HAS_UNIFORM_u_gapwidth
    const mediump float gapwidth = unpack_mix_float(in_gapwidth, drawable.gapwidth_t) / 2.0;
#else
    const mediump float gapwidth = props.gapwidth / 2.0;
#endif

#ifndef HAS_UNIFORM_u_offset
    const lowp float offset = unpack_mix_float(in_offset, drawable.offset_t) * -1.0;
#else
    const lowp float offset = props.offset * -1.0;
#endif

#ifndef HAS_UNIFORM_u_width
    mediump float width = unpack_mix_float(in_width, drawable.width_t);
#else
    mediump float width = props.width;
#endif

#ifndef HAS_UNIFORM_u_pattern_from
    frag_pattern_from = in_pattern_from;
#endif

#ifndef HAS_UNIFORM_u_pattern_to
    frag_pattern_to = in_pattern_to;
#endif
#ifndef HAS_UNIFORM_u_width
    frag_floorwidth = unpack_mix_float(in_width, drawable.floorwidth_t);
#else
    frag_floorwidth = props.floorwidth;
#endif

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;
    const float LINE_DISTANCE_SCALE = 2.0;

    vec2 a_extrude = in_data.xy - 128.0;
    float a_direction = mod(in_data.z, 4.0) - 1.0;
    vec2 pos = floor(in_pos_normal * 0.5);

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of in_pos_normal
    mediump vec2 normal = in_pos_normal - 2.0 * pos;
    frag_normal = vec2(normal.x, normal.y * 2.0 - 1.0);

    // these transformations used to be applied in the JS and native code bases.
    // moved them into the shader for clarity and simplicity.
    float halfwidth = width / 2.0;

    float inset = gapwidth + (gapwidth > 0.0 ? ANTIALIASING : 0.0);
    float outset = gapwidth + halfwidth * (gapwidth > 0.0 ? 2.0 : 1.0) + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);

    // Scale the extrusion vector down to a normal and then up by the line width
    // of this vertex.
    mediump vec2 dist = outset * a_extrude * LINE_NORMAL_SCALE;

    // Calculate the offset when drawing a line that is to the side of the actual line.
    // We do this by creating a vector that points towards the extrude, but rotate
    // it when we're drawing round end points (a_direction = -1 or 1) since their
    // extrude vector points in another direction.
    mediump float u = 0.5 * a_direction;
    mediump float t = 1.0 - abs(u);
    mediump vec2 offset2 = offset * a_extrude * LINE_NORMAL_SCALE * frag_normal.y * mat2(t, -u, u, t);

    vec4 projected_extrude = drawable.matrix * vec4(dist / drawable.ratio, 0.0, 0.0);
    gl_Position = drawable.matrix * vec4(pos + offset2 / drawable.ratio, 0.0, 1.0) + projected_extrude;
    applySurfaceTransform();

    // calculate how much the perspective view squishes or stretches the extrude
    float extrude_length_without_perspective = length(dist);
    float extrude_length_with_perspective = length(projected_extrude.xy / gl_Position.w * paintParams.units_to_pixels);
    frag_gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective;

    frag_width2 = vec2(outset, inset);
    frag_linesofar = (floor(in_data.z / 4.0) + in_data.w * 64.0) * LINE_DISTANCE_SCALE;
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in lowp vec2 frag_normal;
layout(location = 1) in lowp vec2 frag_width2;
layout(location = 2) in lowp float frag_gamma_scale;
layout(location = 3) in float frag_linesofar;

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 4) in lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 5) in lowp float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 6) in mediump vec4 frag_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 7) in mediump vec4 frag_pattern_to;
#endif
layout(location = 8) in mediump float frag_floorwidth;

layout(location = 0) out vec4 out_color;

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct LinePatternTilePropertiesUBO {
    vec4 pattern_from;
    vec4 pattern_to;
    vec4 scale;
    vec2 texsize;
    float fade;
    float pad1;
};

layout(std140, set = LAYER_SET_INDEX, binding = idLineTilePropsUBO) readonly buffer LinePatternTilePropertiesUBOVector {
    LinePatternTilePropertiesUBO tile_props_ubo[];
} tilePropsVector;

layout(set = LAYER_SET_INDEX, binding = idLineEvaluatedPropsUBO) uniform LineEvaluatedPropsUBO {
    vec4 color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;
    uint expressionMask;
    float intersection_rounded_corner_distance;
} props;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image0_sampler;

void main() {

#ifdef OVERDRAW_INSPECTOR
    out_color = vec4(1.0);
    return;
#endif

    const LinePatternTilePropertiesUBO tileProps = tilePropsVector.tile_props_ubo[constant.ubo_index];

#ifdef HAS_UNIFORM_u_blur
    const lowp float blur = props.blur;
#else
    const lowp float blur = frag_blur;
#endif

#ifdef HAS_UNIFORM_u_opacity
    const lowp float opacity = props.opacity;
#else
    const lowp float opacity = frag_opacity;
#endif

#ifdef HAS_UNIFORM_u_pattern_from
    const lowp vec4 pattern_from = tileProps.pattern_from;
#else
    const lowp vec4 pattern_from = frag_pattern_from;
#endif

#ifdef HAS_UNIFORM_u_pattern_to
    const lowp vec4 pattern_to = tileProps.pattern_to;
#else
    const lowp vec4 pattern_to = frag_pattern_to;
#endif

    const vec2 pattern_tl_a = pattern_from.xy;
    const vec2 pattern_br_a = pattern_from.zw;
    const vec2 pattern_tl_b = pattern_to.xy;
    const vec2 pattern_br_b = pattern_to.zw;

    const float pixelRatio = tileProps.scale.x;
    const float tileZoomRatio = tileProps.scale.y;
    const float fromScale = tileProps.scale.z;
    const float toScale = tileProps.scale.w;

    const vec2 display_size_a = vec2((pattern_br_a.x - pattern_tl_a.x) / pixelRatio, (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    const vec2 display_size_b = vec2((pattern_br_b.x - pattern_tl_b.x) / pixelRatio, (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);

    const vec2 pattern_size_a = vec2(display_size_a.x * fromScale / tileZoomRatio, display_size_a.y);
    const vec2 pattern_size_b = vec2(display_size_b.x * toScale / tileZoomRatio, display_size_b.y);

    const float aspect_a = display_size_a.y / frag_floorwidth;
    const float aspect_b = display_size_b.y / frag_floorwidth;

    // Calculate the distance of the pixel from the line in pixels.
    const float dist = length(frag_normal) * frag_width2.x;

    // Calculate the antialiasing fade factor. This is either when fading in
    // the line in case of an offset line (frag_width2.y) or when fading out
    // (frag_width2.x)
    const float blur2 = (blur + 1.0 / DEVICE_PIXEL_RATIO) * frag_gamma_scale;
    const float alpha = clamp(min(dist - (frag_width2.y - blur2), frag_width2.x - dist) / blur2, 0.0, 1.0);

    const float x_a = mod(frag_linesofar / pattern_size_a.x * aspect_a, 1.0);
    const float x_b = mod(frag_linesofar / pattern_size_b.x * aspect_b, 1.0);

    const float y = 0.5 * frag_normal.y + 0.5;

    const vec2 texel_size = 1.0 / tileProps.texsize;
    const vec2 half_texel = 0.5 * texel_size;
    vec2 pos_a = mix(pattern_tl_a * texel_size - texel_size, pattern_br_a * texel_size + texel_size, vec2(x_a, y));
    pos_a = clamp(pos_a, (pattern_tl_a - 1.0) * texel_size + half_texel, (pattern_br_a + 1.0) * texel_size - half_texel);
    vec2 pos_b = mix(pattern_tl_b * texel_size - texel_size, pattern_br_b * texel_size + texel_size, vec2(x_b, y));
    pos_b = clamp(pos_b, (pattern_tl_b - 1.0) * texel_size + half_texel, (pattern_br_b + 1.0) * texel_size - half_texel);

    const vec4 color = mix(texture(image0_sampler, pos_a), texture(image0_sampler, pos_b), tileProps.fade);

    out_color = color * (alpha * opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "LineSDFShader";

    static const std::array<AttributeInfo, 9> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = lineShadePrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_pos_normal;
layout(location = 1) in uvec4 in_data;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 2) in vec4 in_color;
#endif

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 3) in vec2 in_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 4) in vec2 in_opacity;
#endif

#if !defined(HAS_UNIFORM_u_gapwidth)
layout(location = 5) in vec2 in_gapwidth;
#endif

#if !defined(HAS_UNIFORM_u_offset)
layout(location = 6) in vec2 in_offset;
#endif

#if !defined(HAS_UNIFORM_u_width)
layout(location = 7) in vec2 in_width;
#endif

#if !defined(HAS_UNIFORM_u_floorwidth)
layout(location = 8) in vec2 in_floorwidth;
#endif

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct LineSDFDrawableUBO {
    mat4 matrix;
    vec2 patternscale_a;
    vec2 patternscale_b;
    float tex_y_a;
    float tex_y_b;
    float ratio;
    // Interpolations
    float color_t;
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float floorwidth_t;
    float pad1;
    float pad2;
};

layout(std140, set = LAYER_SET_INDEX, binding = idLineDrawableUBO) readonly buffer LineSDFDrawableUBOVector {
    LineSDFDrawableUBO drawable_ubo[];
} drawableVector;

layout(set = LAYER_SET_INDEX, binding = idLineEvaluatedPropsUBO) uniform LineEvaluatedPropsUBO {
    vec4 color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;
    uint expressionMask;
    float intersection_rounded_corner_distance;
} props;

layout(location = 0) out lowp vec2 frag_normal;
layout(location = 1) out lowp vec2 frag_width2;
layout(location = 2) out lowp float frag_gamma_scale;
layout(location = 3) out vec2 frag_tex_a;
layout(location = 4) out vec2 frag_tex_b;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 5) out lowp vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 6) out lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 7) out lowp float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_floorwidth)
layout(location = 8) out mediump float frag_floorwidth;
#endif

void main() {
    const LineSDFDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

#ifndef HAS_UNIFORM_u_color
    frag_color = unpack_mix_color(in_color, drawable.color_t);
#endif

#ifndef HAS_UNIFORM_u_blur
    frag_blur = unpack_mix_float(in_blur, drawable.blur_t);
#endif

#ifndef HAS_UNIFORM_u_opacity
    frag_opacity = unpack_mix_float(in_opacity, drawable.opacity_t);
#endif

#ifndef HAS_UNIFORM_u_floorwidth
    const float floorwidth = unpack_mix_float(in_floorwidth, drawable.floorwidth_t);
    frag_floorwidth = floorwidth;
#else
    const float floorwidth = props.floorwidth;
#endif

#ifndef HAS_UNIFORM_u_offset
    const mediump float offset = unpack_mix_float(in_offset, drawable.offset_t) * -1.0;
#else
    const mediump float offset = props.offset * -1.0;
#endif

#ifndef HAS_UNIFORM_u_width
    const mediump float width = unpack_mix_float(in_width, drawable.width_t);
#else
    const mediump float width = props.width;
#endif

#ifndef HAS_UNIFORM_u_gapwidth
    const mediump float gapwidth = unpack_mix_float(in_gapwidth, drawable.gapwidth_t) / 2.0;
#else
    const mediump float gapwidth = props.gapwidth / 2.0;
#endif

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;
    const float LINE_DISTANCE_SCALE = 2.0;

    vec2 a_extrude = in_data.xy - 128.0;
    float a_direction = mod(in_data.z, 4.0) - 1.0;
    float linesofar = (floor(in_data.z / 4.0) + in_data.w * 64.0) * LINE_DISTANCE_SCALE;
    vec2 pos = floor(in_pos_normal * 0.5);

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of in_pos_normal
    mediump vec2 normal = in_pos_normal - 2.0 * pos;
    frag_normal = vec2(normal.x, normal.y * 2.0 - 1.0);

    // these transformations used to be applied in the JS and native code bases.
    // moved them into the shader for clarity and simplicity.
    float halfwidth = width / 2.0;

    float inset = gapwidth + (gapwidth > 0.0 ? ANTIALIASING : 0.0);
    float outset = gapwidth + halfwidth * (gapwidth > 0.0 ? 2.0 : 1.0) + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);

    // Scale the extrusion vector down to a normal and then up by the line width
    // of this vertex.
    mediump vec2 dist = outset * a_extrude * LINE_NORMAL_SCALE;

    // Calculate the offset when drawing a line that is to the side of the actual line.
    // We do this by creating a vector that points towards the extrude, but rotate
    // it when we're drawing round end points (a_direction = -1 or 1) since their
    // extrude vector points in another direction.
    mediump float u = 0.5 * a_direction;
    mediump float t = 1.0 - abs(u);
    mediump vec2 offset2 = offset * a_extrude * LINE_NORMAL_SCALE * frag_normal.y * mat2(t, -u, u, t);

    vec4 projected_extrude = drawable.matrix * vec4(dist / drawable.ratio, 0.0, 0.0);
    gl_Position = drawable.matrix * vec4(pos + offset2 / drawable.ratio, 0.0, 1.0) + projected_extrude;
    applySurfaceTransform();

    // calculate how much the perspective view squishes or stretches the extrude
    float extrude_length_without_perspective = length(dist);
    float extrude_length_with_perspective = length(projected_extrude.xy / gl_Position.w * paintParams.units_to_pixels);
    frag_gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective;

    frag_width2 = vec2(outset, inset);

    frag_tex_a = vec2(linesofar * drawable.patternscale_a.x / floorwidth, frag_normal.y * drawable.patternscale_a.y + drawable.tex_y_a);
    frag_tex_b = vec2(linesofar * drawable.patternscale_b.x / floorwidth, frag_normal.y * drawable.patternscale_b.y + drawable.tex_y_b);
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in lowp vec2 frag_normal;
layout(location = 1) in lowp vec2 frag_width2;
layout(location = 2) in lowp float frag_gamma_scale;
layout(location = 3) in vec2 frag_tex_a;
layout(location = 4) in vec2 frag_tex_b;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 5) in lowp vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 6) in lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 7) in lowp float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_floorwidth)
layout(location = 8) in mediump float frag_floorwidth;
#endif

layout(location = 0) out vec4 out_color;

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct LineSDFTilePropsUBO {
    float sdfgamma;
    float mix;
    float pad1;
    float pad2;
    vec4 pad3;
    vec4 pad4;
    vec4 pad5;
};

layout(std140, set = LAYER_SET_INDEX, binding = idLineTilePropsUBO) readonly buffer LineSDFTilePropsUBOVector {
    LineSDFTilePropsUBO tile_props_ubo[];
} tilePropsVector;

layout(set = LAYER_SET_INDEX, binding = idLineEvaluatedPropsUBO) uniform LineEvaluatedPropsUBO {
    vec4 color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;
    uint expressionMask;
    float intersection_rounded_corner_distance;
} props;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image0_sampler;

void main() {

#ifdef OVERDRAW_INSPECTOR
    out_color = vec4(1.0);
    return;
#endif

    const LineSDFTilePropsUBO tileProps = tilePropsVector.tile_props_ubo[constant.ubo_index];

#ifdef HAS_UNIFORM_u_color
    const lowp vec4 color = props.color;
#else
    const lowp vec4 color = frag_color;
#endif

#ifdef HAS_UNIFORM_u_blur
    const lowp float blur = props.blur;
#else
    const lowp float blur = frag_blur;
#endif

#ifdef HAS_UNIFORM_u_opacity
    const lowp float opacity = props.opacity;
#else
    const lowp float opacity = frag_opacity;
#endif

#ifdef HAS_UNIFORM_u_floorwidth
    const lowp float floorwidth = props.floorwidth;
#else
    const lowp float floorwidth = frag_floorwidth;
#endif

    // Calculate the distance of the pixel from the line in pixels.
    const float dist = length(frag_normal) * frag_width2.x;

    // Calculate the antialiasing fade factor. This is either when fading in the
    // line in case of an offset line (`v_width2.y`) or when fading out (`v_width2.x`)
    const float blur2 = (blur + 1.0 / DEVICE_PIXEL_RATIO) * frag_gamma_scale;

    const float sdfdist_a = texture(image0_sampler, frag_tex_a).a;
    const float sdfdist_b = texture(image0_sampler, frag_tex_b).a;
    const float sdfdist = mix(sdfdist_a, sdfdist_b, tileProps.mix);
    const float alpha = clamp(min(dist - (frag_width2.y - blur2), frag_width2.x - dist) / blur2, 0.0, 1.0) *
                        smoothstep(0.5 - tileProps.sdfgamma / floorwidth, 0.5 + tileProps.sdfgamma / floorwidth, sdfdist);

    out_color = color * (alpha * opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::LineGradientSDFShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "LineGradientSDFShader";

    static const std::array<AttributeInfo, 8> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;

    static constexpr auto prelude = lineShadePrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_pos_normal;
layout(location = 1) in uvec4 in_data;

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 2) in vec2 in_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 3) in vec2 in_opacity;
#endif

#if !defined(HAS_UNIFORM_u_gapwidth)
layout(location = 4) in vec2 in_gapwidth;
#endif

#if !defined(HAS_UNIFORM_u_offset)
layout(location = 5) in vec2 in_offset;
#endif

#if !defined(HAS_UNIFORM_u_width)
layout(location = 6) in vec2 in_width;
#endif

#if !defined(HAS_UNIFORM_u_floorwidth)
layout(location = 7) in vec2 in_floorwidth;
#endif

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct LineSDFDrawableUBO {
    mat4 matrix;
    vec2 patternscale_a;
    vec2 patternscale_b;
    float tex_y_a;
    float tex_y_b;
    float ratio;
    // Interpolations
    float color_t;
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float floorwidth_t;
    float pad1;
    float pad2;
};

layout(std140, set = LAYER_SET_INDEX, binding = idLineDrawableUBO) readonly buffer LineSDFDrawableUBOVector {
    LineSDFDrawableUBO drawable_ubo[];
} drawableVector;

layout(set = LAYER_SET_INDEX, binding = idLineEvaluatedPropsUBO) uniform LineEvaluatedPropsUBO {
    vec4 color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;
    uint expressionMask;
    float intersection_rounded_corner_distance;
} props;

layout(location = 0) out lowp vec2 frag_normal;
layout(location = 1) out lowp vec2 frag_width2;
layout(location = 2) out lowp float frag_gamma_scale;
layout(location = 3) out vec2 frag_tex_a;
layout(location = 4) out vec2 frag_tex_b;
layout(location = 5) out float frag_lineprogress;

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 6) out lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 7) out lowp float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_floorwidth)
layout(location = 8) out mediump float frag_floorwidth;
#endif

void main() {
    const LineSDFDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

#ifndef HAS_UNIFORM_u_blur
    frag_blur = unpack_mix_float(in_blur, drawable.blur_t);
#endif

#ifndef HAS_UNIFORM_u_opacity
    frag_opacity = unpack_mix_float(in_opacity, drawable.opacity_t);
#endif

#ifndef HAS_UNIFORM_u_floorwidth
    const float floorwidth = unpack_mix_float(in_floorwidth, drawable.floorwidth_t);
    frag_floorwidth = floorwidth;
#else
    const float floorwidth = props.floorwidth;
#endif

#ifndef HAS_UNIFORM_u_offset
    const mediump float offset = unpack_mix_float(in_offset, drawable.offset_t) * -1.0;
#else
    const mediump float offset = props.offset * -1.0;
#endif

#ifndef HAS_UNIFORM_u_width
    const mediump float width = unpack_mix_float(in_width, drawable.width_t);
#else
    const mediump float width = props.width;
#endif

#ifndef HAS_UNIFORM_u_gapwidth
    const mediump float gapwidth = unpack_mix_float(in_gapwidth, drawable.gapwidth_t) / 2.0;
#else
    const mediump float gapwidth = props.gapwidth / 2.0;
#endif

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;
    const float LINE_DISTANCE_SCALE = 2.0;

    vec2 a_extrude = in_data.xy - 128.0;
    float a_direction = mod(in_data.z, 4.0) - 1.0;
    float linesofar = (floor(in_data.z / 4.0) + in_data.w * 64.0) * LINE_DISTANCE_SCALE;
    frag_lineprogress = (floor(in_data.z / 4.0) + in_data.w * 64.0) * 2.0 / MAX_LINE_DISTANCE;
    vec2 pos = floor(in_pos_normal * 0.5);

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of in_pos_normal
    mediump vec2 normal = in_pos_normal - 2.0 * pos;
    frag_normal = vec2(normal.x, normal.y * 2.0 - 1.0);

    // these transformations used to be applied in the JS and native code bases.
    // moved them into the shader for clarity and simplicity.
    float halfwidth = width / 2.0;

    float inset = gapwidth + (gapwidth > 0.0 ? ANTIALIASING : 0.0);
    float outset = gapwidth + halfwidth * (gapwidth > 0.0 ? 2.0 : 1.0) + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);

    // Scale the extrusion vector down to a normal and then up by the line width
    // of this vertex.
    mediump vec2 dist = outset * a_extrude * LINE_NORMAL_SCALE;

    // Calculate the offset when drawing a line that is to the side of the actual line.
    // We do this by creating a vector that points towards the extrude, but rotate
    // it when we're drawing round end points (a_direction = -1 or 1) since their
    // extrude vector points in another direction.
    mediump float u = 0.5 * a_direction;
    mediump float t = 1.0 - abs(u);
    mediump vec2 offset2 = offset * a_extrude * LINE_NORMAL_SCALE * frag_normal.y * mat2(t, -u, u, t);

    vec4 projected_extrude = drawable.matrix * vec4(dist / drawable.ratio, 0.0, 0.0);
    gl_Position = drawable.matrix * vec4(pos + offset2 / drawable.ratio, 0.0, 1.0) + projected_extrude;
    applySurfaceTransform();

    // calculate how much the perspective view squishes or stretches the extrude
    float extrude_length_without_perspective = length(dist);
    float extrude_length_with_perspective = length(projected_extrude.xy / gl_Position.w * paintParams.units_to_pixels);
    frag_gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective;

    frag_width2 = vec2(outset, inset);

    frag_tex_a = vec2(linesofar * drawable.patternscale_a.x / floorwidth, frag_normal.y * drawable.patternscale_a.y + drawable.tex_y_a);
    frag_tex_b = vec2(linesofar * drawable.patternscale_b.x / floorwidth, frag_normal.y * drawable.patternscale_b.y + drawable.tex_y_b);
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in lowp vec2 frag_normal;
layout(location = 1) in lowp vec2 frag_width2;
layout(location = 2) in lowp float frag_gamma_scale;
layout(location = 3) in vec2 frag_tex_a;
layout(location = 4) in vec2 frag_tex_b;
layout(location = 5) in float frag_lineprogress;

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 6) in lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 7) in lowp float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_floorwidth)
layout(location = 8) in mediump float frag_floorwidth;
#endif

layout(location = 0) out vec4 out_color;

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct LineSDFTilePropsUBO {
    float sdfgamma;
    float mix;
    float pad1;
    float pad2;
    vec4 pad3;
    vec4 pad4;
    vec4 pad5;
};

layout(std140, set = LAYER_SET_INDEX, binding = idLineTilePropsUBO) readonly buffer LineSDFTilePropsUBOVector {
    LineSDFTilePropsUBO tile_props_ubo[];
} tilePropsVector;

layout(set = LAYER_SET_INDEX, binding = idLineEvaluatedPropsUBO) uniform LineEvaluatedPropsUBO {
    vec4 color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;
    uint expressionMask;
    float intersection_rounded_corner_distance;
} props;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image0_sampler;
layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 1) uniform sampler2D image1_sampler;

void main() {

#ifdef OVERDRAW_INSPECTOR
    out_color = vec4(1.0);
    return;
#endif

    const LineSDFTilePropsUBO tileProps = tilePropsVector.tile_props_ubo[constant.ubo_index];

#ifdef HAS_UNIFORM_u_blur
    const lowp float blur = props.blur;
#else
    const lowp float blur = frag_blur;
#endif

#ifdef HAS_UNIFORM_u_opacity
    const lowp float opacity = props.opacity;
#else
    const lowp float opacity = frag_opacity;
#endif

#ifdef HAS_UNIFORM_u_floorwidth
    const lowp float floorwidth = props.floorwidth;
#else
    const lowp float floorwidth = frag_floorwidth;
#endif

    // Calculate the distance of the pixel from the line in pixels.
    const float dist = length(frag_normal) * frag_width2.x;

    // Calculate the antialiasing fade factor. This is either when fading in the
    // line in case of an offset line (`v_width2.y`) or when fading out (`v_width2.x`)
    const float blur2 = (blur + 1.0 / DEVICE_PIXEL_RATIO) * frag_gamma_scale;

    const float sdfdist_a = texture(image1_sampler, frag_tex_a).a;
    const float sdfdist_b = texture(image1_sampler, frag_tex_b).a;
    const float sdfdist = mix(sdfdist_a, sdfdist_b, tileProps.mix);
    const float alpha = clamp(min(dist - (frag_width2.y - blur2), frag_width2.x - dist) / blur2, 0.0, 1.0) *
                        smoothstep(0.5 - tileProps.sdfgamma / floorwidth, 0.5 + tileProps.sdfgamma / floorwidth, sdfdist);

    // The gradient ramp is stored in a texture indexed by the ratio along the entire line.
    const vec4 color = texture(image0_sampler, vec2(frag_lineprogress, 0.5));

    out_color = color * (alpha * opacity);
}
)";
};

} // namespace shaders
} // namespace mln
