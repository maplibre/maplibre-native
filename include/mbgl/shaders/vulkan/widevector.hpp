#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto wideVectorShaderPrelude = R"(

#define idWideVectorUniformsUBO         idDrawableReservedVertexOnlyUBO
#define idWideVectorUniformWideVecUBO   drawableReservedUBOCount

)";

template <>
struct ShaderSource<BuiltIn::WideVectorShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "WideVectorShader";

    static const std::array<AttributeInfo, 3> attributes;
    static const std::array<AttributeInfo, 4> instanceAttributes;
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = wideVectorShaderPrelude;
    static constexpr auto vertex = R"(

/** Expressions are used to change values like width and opacity over zoom levels. **/
#define WKSExpStops 8

// Line Joins
// These are assumed to match WideVectorLineJoinType
#define WKSVertexLineJoinMiter          0
#define WKSVertexLineJoinBevel          1
#define WKSVertexLineJoinRound          2
#define WKSVertexLineJoinMiterClip      3
#define WKSVertexLineJoinMiterSimple    4
#define WKSVertexLineJoinNone           5

// Line Caps
// These are assumed to match WideVectorLineCapType
#define WKSVertexLineCapRound   0
#define WKSVertexLineCapButt    1
#define WKSVertexLineCapSquare  2

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idWideVectorUniformsUBO) uniform WideVectorUniformsUBO {
    mat4 mvpMatrix;
    mat4 mvpMatrixDiff;
    mat4 mvMatrix;
    mat4 mvMatrixDiff;
    mat4 pMatrix;
    mat4 pMatrixDiff;
    vec2 frameSize;
} uniforms;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idWideVectorUniformWideVecUBO) uniform WideVectorUniformWideVecUBO {
    vec4 color;
    float w2;
    float offset;
    float edge;
    float texRepeat;
    vec2 texOffset;
    float miterLimit;
    int join;
    int cap;
    int hasExp;
    float interClipLimit;
} wideVec;

// Instance info for the wide vector (new) vertex shader
typedef struct
{
    // Center of the point on the line
    vec3 center;
    // Color
    vec4 color;
    // Used to track loops and such
    int prev,next; // set to -1 for non-loops
} VertexTriWideVecInstance;

struct IntersectInfo {
    bool valid;
    vec2 interPt;
    float c;
    float ta, tb;
};

// wedge product (2D cross product)
// if A ^ B > 0, A is to the right of B
float wedge(vec2 a, vec2 b) {
    return a.x * b.y - a.y * b.x;
}

// Intersect two lines
IntersectInfo intersectLines(vec2 a0, vec2 a1, vec2 b0, vec2 b1)
{
    const vec2 dA = a0 - a1;
    const vec2 dB = b0 - b1;

    // Solve the system of equations formed by equating the lines to get their common point.
    const float denom = wedge(dA, dB);
    if (denom == 0.0) {
        // If the denominator comes out zero, the lines are parallel and do not intersect
        return { .valid = false };
    }

    const float tA = wedge(a0, a1);
    const float tB = wedge(b0, b1);
    const vec2 inter = vec2((tA * dB.x - dA.x * tB), (tA * dB.y - dA.y * tB)) / denom;

    return {
        .valid = true,
        .interPt = inter,
        .ta = 0.0,
        .tb = 0.0,
    };
}

// Intersect two offset lines
IntersectInfo intersectWideLines(vec2 p0,vec2 p1,vec2 p2, vec2 n0,vec2 n1)
{
    return intersectLines(p0 + n0, p1 + n0, p1 + n1, p2 + n1);
}

// Used to track what info we have about a center point
struct CenterInfo {
    /// Screen coordinates of the line segment endpoint
    vec2 screenPos;
    /// Length of the segment (in screen coordinates)
    float len;
    /// Normalized direction of the segment
    vec2 nDir;
    /// Normalized plane normal, perpendicular to the segment
    vec2 norm;
};

vec3 viewPos(const mat4 &mat, vec3 vec) {
    const vec4 p = mat * vec4(vec, 1.0);
    return p.xyz;   // / p.w; ?
}

vec2 screenPos_MVP(const Uniforms &u, vec3 viewPos) {
    const vec4 p4 = vec4(viewPos, 1.0);

    // Use the MVP matrix
    const vec4 s = u.mvpMatrix * p4;

    return s.xy / s.w;
}

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in int in_index;

layout(location = 3) in vec3 in_instance_center;
layout(location = 4) in vec4 in_instance_color;
layout(location = 5) in int in_instance_prev;
layout(location = 6) in int in_instance_next;

layout(location = 0) out vec2 frag_screenPos;       // un-transformed vertex position
layout(location = 1) out vec2 frag_centerPos;       // un-transformed circle center
layout(location = 2) out vec2 frag_midDir;          // Turn direction
layout(location = 3) out vec4 frag_color;
layout(location = 4) out vec2 frag_texCoord;
layout(location = 5) out float frag_w2;
layout(location = 6) out float frag_edge;
layout(location = 7) out uvec2 frag_maskIDs;
layout(location = 8) out bool frag_roundJoin;

const float wideVecMinTurnThreshold = 1e-5;
const float wideVecMaxTurnThreshold = 0.99999998476;  // sin(89.99 deg)
const int WideVecPolyStartGeom = 0;
const int WideVecPolyBodyGeom = 1;
const int WideVecPolyEndGeom = 2;
const vec4 discardPt = vec4(0,0,-1e6,NAN);

void main() {


}
)";

    static constexpr auto fragment = R"(
    // TODO
)";
};

} // namespace shaders
} // namespace mbgl
