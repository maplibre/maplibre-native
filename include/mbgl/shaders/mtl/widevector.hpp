#pragma once

#include <mbgl/shaders/widevector_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto wideVectorShaderPrelude = R"(

enum {
    idWideVectorUniformsUBO = idDrawableReservedVertexOnlyUBO,
    idWideVectorUniformWideVecUBO = drawableReservedUBOCount,
    wideVectorUBOCount
};

)";

template <>
struct ShaderSource<BuiltIn::WideVectorShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "WideVectorShader";
    static constexpr auto vertexMainFunction = "vertexTri_wideVecPerf";
    static constexpr auto fragmentMainFunction = "fragmentTri_wideVecPerf";

    static const std::array<AttributeInfo, 3> attributes;
    static const std::array<AttributeInfo, 4> instanceAttributes;
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = wideVectorShaderPrelude;
    static constexpr auto source = R"(

namespace WhirlyKitShader
{

/** Expressions are used to change values like width and opacity over zoom levels. **/
#define WKSExpStops 8

// Line Joins
// These are assumed to match WideVectorLineJoinType
typedef enum {
    WKSVertexLineJoinMiter = 0,
    WKSVertexLineJoinBevel = 1,
    WKSVertexLineJoinRound = 2,
    WKSVertexLineJoinMiterClip = 3,
    WKSVertexLineJoinMiterSimple = 4,
    WKSVertexLineJoinNone = 5,
} WKSVertexLineJoinType;

// Line Caps
// These are assumed to match WideVectorLineCapType
typedef enum {
    WKSVertexLineCapRound = 0,
    WKSVertexLineCapButt = 1,
    WKSVertexLineCapSquare = 2,
} WKSVertexLineCapType;

// Uniforms for the basic case.
struct Uniforms
{
    simd::float4x4 mvpMatrix;
    simd::float4x4 mvpMatrixDiff;
    simd::float4x4 mvMatrix;
    simd::float4x4 mvMatrixDiff;
    simd::float4x4 pMatrix;
    simd::float4x4 pMatrixDiff;
    simd::float2 frameSize;
};

// Instructions to the wide vector shaders, usually per-drawable
struct UniformWideVec {
    simd::float4 color;          // Color
    float w2;                    // Width / 2.0 in screen space
    float offset;                // Offset from center in screen space
    float edge;                  // Edge falloff control
    float texRepeat;             // Texture scaling specific to wide vectors
    simd::float2 texOffset;      // Texture offset.
    float miterLimit;            // Miter join limit, multiples of width
    WKSVertexLineJoinType join;  // Line joins
    WKSVertexLineCapType cap;    // Line endcaps
    bool hasExp;                 // Look for a UniformWideVecExp structure for color, opacity, and width
    float interClipLimit;        // Allow clipping of out-of-bounds intersection points
                                 // Value is the multiple of distance-squared that is allowed.
};

// Instance info for the wide vector (new) vertex shader
typedef struct
{
    // Center of the point on the line
    simd::float3 center;
    // Color
    simd::float4 color;
    // Used to track loops and such
    int prev,next; // set to -1 for non-loops
} VertexTriWideVecInstance;

}

// These have syntax that only the shader language can grok
// We put them here so we can include them in other Metal libraries
#ifdef __METAL_VERSION__

/**
 Wide Vector Shaders
 These work to build/render objects in 2D space, but based
 on 3D locations.
 */

// Vertex definition for wide vector (new version)
struct VertexTriWideVecB
{
    // x, y offset around the center
    float3 screenPos [[attribute(wideVectorUBOCount + 0)]];
    float4 color [[attribute(wideVectorUBOCount + 1)]];
    int index [[attribute(wideVectorUBOCount + 2)]];
};

// Wide vector vertex passed to fragment shader (new version)
struct ProjVertexTriWideVecPerf {
    float4 position [[invariant]] [[position]];     // transformed to NDC
    float2 screenPos;                               // un-transformed vertex position
    float2 centerPos;                               // un-transformed circle center
    float2 midDir;                                  // Turn direction
    float4 color;
    float2 texCoord;
    float w2;
    float edge;
    uint2 maskIDs;
    bool roundJoin;

    //uint whichVert;       // helpful for debugging
};

#else

#endif

using namespace metal;
using namespace WhirlyKitShader;

struct IntersectInfo {
    bool valid;
    float2 interPt;
    float c;
    float ta,tb;
};

// wedge product (2D cross product)
// if A ^ B > 0, A is to the right of B
float wedge(float2 a, float2 b) {
    return a.x * b.y - a.y * b.x;
}

// Intersect two lines
IntersectInfo intersectLines(float2 a0, float2 a1, float2 b0, float2 b1)
{
    const float2 dA = a0 - a1;
    const float2 dB = b0 - b1;

    // Solve the system of equations formed by equating the lines to get their common point.
    const float denom = wedge(dA, dB);
    if (denom == 0.0) {
        // If the denominator comes out zero, the lines are parallel and do not intersect
        return { .valid = false };
    }

    const float tA = wedge(a0, a1);
    const float tB = wedge(b0, b1);
    const float2 inter = float2((tA * dB.x - dA.x * tB), (tA * dB.y - dA.y * tB)) / denom;

    return {
        .valid = true,
        .interPt = inter,
        .ta = 0.0,
        .tb = 0.0,
    };
}

// Intersect two offset lines
IntersectInfo intersectWideLines(float2 p0,float2 p1,float2 p2,
                                 float2 n0,float2 n1)
{
    return intersectLines(p0 + n0, p1 + n0, p1 + n1, p2 + n1);
}

// Used to track what info we have about a center point
struct CenterInfo {
    /// Screen coordinates of the line segment endpoint
    float2 screenPos;
    /// Length of the segment (in screen coordinates)
    float len;
    /// Normalized direction of the segment
    float2 nDir;
    /// Normalized plane normal, perpendicular to the segment
    float2 norm;
};

float3 viewPos(constant simd::float4x4 &mat, float3 vec) {
    const float4 p = mat * float4(vec,1.0);
    return p.xyz;   // / p.w; ?
}

float2 screenPos_MVP(constant Uniforms &u, float3 viewPos) {
    const float4 p4 = float4(viewPos, 1.0);

    // Use the MVP matrix
    const float4 s = u.mvpMatrix * p4;

    return s.xy/s.w;
}

constant constexpr float wideVecMinTurnThreshold = 1e-5;
constant constexpr float wideVecMaxTurnThreshold = 0.99999998476;  // sin(89.99 deg)
constant constexpr int WideVecPolyStartGeom = 0;
constant constexpr int WideVecPolyBodyGeom = 1;
constant constexpr int WideVecPolyEndGeom = 2;
constant constexpr float4 discardPt(0,0,-1e6,NAN);

// Performance version of wide vector shader
vertex ProjVertexTriWideVecPerf vertexTri_wideVecPerf(
          thread const VertexTriWideVecB vert [[ stage_in ]],
          constant Uniforms &uniforms [[ buffer(idWideVectorUniformsUBO) ]],
          constant UniformWideVec &wideVec [[ buffer(idWideVectorUniformWideVecUBO) ]],
          uint instanceID [[ instance_id ]],
          constant VertexTriWideVecInstance *wideVecInsts   [[ buffer(wideVectorUBOCount + 3) ]])
{
    ProjVertexTriWideVecPerf outVert = {
        .position = discardPt,
        .roundJoin = false,
    };

    // Vertex index within the instance, 0-11
    // Odd indexes are on the left, evens are on the right.
    const int whichVert = (vert.index >> 16) & 0xffff;
    // Polygon index within the segment.  0=Start cap, 1=body, 2=end cap
    const int whichPoly = vert.index & 0xffff;
    // Are we on the left edge, or the right?
    const bool isLeft = (whichVert & 1);
    // Are we on the starting end of the segment or the end?
    const bool isEnd = (whichVert > 5);

    // Track vertex for debugging
    //outVert.whichVert = whichVert;

    // Pull out the width and possibly calculate one
    float w2 = wideVec.w2;

    // w2 includes edge-blend, strokeWidth does not
    const float strokeWidth = 2 * w2;
    if (w2 > 0.0) {
        w2 = w2 + wideVec.edge;
    }

    // Disable joins for narrow lines
    auto joinType = (w2 >= 1) ? wideVec.join : WKSVertexLineJoinNone;

    // Find the various instances representing center points.
    // We need one behind and one ahead of us.
    //                     previous segment
    //                     |      this segment
    //                     |      |    next segment
    //                     |      |    |
    bool instValid[4] = { false, true, false, false };
    VertexTriWideVecInstance inst[4] = { {}, wideVecInsts[instanceID], {}, {} };

    if (inst[1].prev != -1 && joinType != WKSVertexLineJoinNone) {
        inst[0] = wideVecInsts[inst[1].prev];
        instValid[0] = true;
    }
    if (inst[1].next != -1) {
        inst[2] = wideVecInsts[inst[1].next];
        instValid[2] = true;

        if (inst[2].next != -1 && joinType != WKSVertexLineJoinNone) {
            inst[3] = wideVecInsts[inst[2].next];
            instValid[3] = true;
        }
    } else {
        // We need at least this and next
        return outVert;
    }

    const auto capType = wideVec.cap;
    const auto isStartCap = (whichPoly == WideVecPolyStartGeom && !instValid[0]);
    const auto isEndCap = (whichPoly == WideVecPolyEndGeom && !instValid[3]);

    // Butt is the default cap style, the line ends at the point.
    if ((isStartCap || isEndCap) && capType == WhirlyKitShader::WKSVertexLineCapButt) {
        return outVert;
    }

    // Figure out position on the screen for each center point.
    // centers[1] represents the segment leading to the current point.
    // centers[2] represents the segment leading from the current point.
    // centers[X] = vector from centers[X-1] to centers[X]
    CenterInfo centers[4];
    for (unsigned int ii=0;ii<4;ii++) {
        if (!instValid[ii]) {
            continue;
        }
        centers[ii].screenPos = screenPos_MVP(uniforms, inst[ii].center);
    }

    const float2 screenScale(2.0/uniforms.frameSize.x,2.0/uniforms.frameSize.y);    // ~(0.001,0.001)

    // Calculate directions and normals.  Done in isotropic coords to
    // avoid skewing everything when later multiplying by `screenScale`.
    for (unsigned int ii=1;ii<4;ii++) {
        if (instValid[ii-1]) {
            const float2 scaledDir = (centers[ii].screenPos - centers[ii-1].screenPos) / screenScale;
            centers[ii].len = length(scaledDir);
            centers[ii].nDir = normalize(scaledDir);
            centers[ii].norm = float2(-centers[ii].nDir.y, centers[ii].nDir.x);
        }
    }

    // Pull out the center line offset, or calculate one
    float centerLine = wideVec.offset;

    // Intersect on the left or right depending
    const float interSgn = (whichVert & 1) ? 1 : -1;

    // Do the offset intersection
    bool intersectValid = false;
    bool turningLeft = false;
    const int interIdx = isEnd ? 1 : 0;
    float2 offsetCenter = centers[interIdx + 1].screenPos + centers[2].norm * centerLine * screenScale;
    float2 interPt(0, 0), realInterPt(0, 0);
    float dotProd = 0, theta = 0, miterLength = 0;

    // If we're on the far end of the body segment, we need this and the next two segments.
    // Otherwise we need the previous, this, and the next segment.
    if (instValid[interIdx] && instValid[interIdx+1] && instValid[interIdx+2]) {

        // Don't even bother computing intersections for very acute angles or very small turns
        dotProd = dot(centers[interIdx+1].nDir, centers[interIdx+2].nDir);
        if (-wideVecMaxTurnThreshold < dotProd &&
            dotProd < wideVecMaxTurnThreshold &&
            abs(abs(dotProd) - 1) >= wideVecMinTurnThreshold) {

            // Interior turn angle, both vectors are normalized.
            theta = M_PI_F - acos(dotProd);

            // "If the miter length divided by the stroke width exceeds the miterlimit then:
            //   miter: the join is converted to a bevel
            //   miter-clip: the miter is clipped at half the miter length from the intersection"
            if (joinType == WKSVertexLineJoinMiter || joinType == WKSVertexLineJoinMiterClip) {
                miterLength = abs(1 / sin(theta / 2));
                if (miterLength > wideVec.miterLimit) {
                    if (joinType == WKSVertexLineJoinMiter) {
                        joinType = WKSVertexLineJoinBevel;
                    } else if (joinType == WKSVertexLineJoinMiterClip) {
                        miterLength = wideVec.miterLimit;
                    }
                }
            }

            // Intersect the left or right sides of prev-this and this-next, plus offset
            thread const CenterInfo &prev = centers[interIdx+0];
            thread const CenterInfo &cur  = centers[interIdx+1];
            thread const CenterInfo &next = centers[interIdx+2];
            const float2 edgeDist = screenScale * (interSgn * w2 + centerLine);
            const float2 n0 = edgeDist * cur.norm;
            const float2 n1 = edgeDist * next.norm;
            const IntersectInfo interInfo = intersectLines(prev.screenPos + n0, cur.screenPos + n0,
                                                           cur.screenPos + n1, next.screenPos + n1);
            if (interInfo.valid) {
                const float c = wedge(next.screenPos - cur.screenPos, cur.screenPos - prev.screenPos);
                turningLeft = (c < 0);
                realInterPt = interPt = interInfo.interPt;

                // Limit the distance to the smaller of half way back along the previous segment
                // or half way forward along the next one to keep consecutive segments from colliding.
                const float maxAdjDist = min(cur.len, next.len) / 2;

                // If we're using offsets, we also need to know the point where the offset lines
                // intersect, as the distance to the original intersection point isn't helpful.
                // todo: this doesn't make sense for small angles, but what's the actual threshold?
                if (centerLine != 0) {
                    const float2 cn0 = cur.norm * centerLine * screenScale;
                    const float2 cn1 = next.norm * centerLine * screenScale;
                    const IntersectInfo i2 = intersectLines(prev.screenPos + cn0, cur.screenPos + cn0,
                                                            cur.screenPos + cn1, next.screenPos + cn1);
                    if (i2.valid && length_squared((i2.interPt - offsetCenter)/screenScale) < maxAdjDist*maxAdjDist) {
                        offsetCenter = i2.interPt;
                    }
                }

                const float2 interVec = (interPt - offsetCenter) / screenScale;
                const float interDist2 = length_squared(interVec);
                const float maxClipDist2 = (maxAdjDist * wideVec.interClipLimit) *
                                           (maxAdjDist * wideVec.interClipLimit);

                // Limit intersection distance.
                // For up to a multiple of that, adjust the intersection point back along its
                // length to that limit, effectively narrowing the line instead of just giving up.
                if (interDist2 <= maxAdjDist*maxAdjDist) {
                    intersectValid = true;
                } else if (wideVec.interClipLimit > 0 && interDist2 <= maxClipDist2) {
                    interPt = offsetCenter + normalize(interVec) * sqrt(interDist2) * screenScale;
                    intersectValid = true;
                }
            }
        }
    }

    // Endcaps not used for miter case, discard them.
    if ((joinType == WKSVertexLineJoinMiter || joinType == WKSVertexLineJoinMiterSimple) &&
        whichPoly != WideVecPolyBodyGeom && !isStartCap && !isEndCap) {
        return outVert;
    }

    // Get the color from the uniform:
    float4 color = wideVec.color;

    outVert.color = color;
    outVert.w2 = w2;
    outVert.edge = wideVec.edge;

    // Work out the corner positions by extending the normals
    const float2 realEdge = interSgn * w2 * screenScale;
    const float2 corner = offsetCenter + centers[2].norm * realEdge;
    const float turnSgn = turningLeft ? -1 : 1;
    const bool isInsideEdge = (isLeft == turningLeft);

    // Current corner is the default result for all points.
    float2 pos = corner;

    bool discardTri = false;

    if (isStartCap || isEndCap) {
        // Square extends beyond the point by half a width.
        // Round uses the same geometry but rounds it off with the fragment shader.
        if (capType == WhirlyKitShader::WKSVertexLineCapSquare ||
            capType == WhirlyKitShader::WKSVertexLineCapRound) {
            switch (whichVert) {
                case 0: case 1: case 10: case 11:
                    pos = corner + centers[2].nDir * w2 * screenScale * (isEnd ? 1 : -1);
                    break;
            }
        }
        if (capType == WhirlyKitShader::WKSVertexLineCapRound) {
            outVert.roundJoin = true;
            outVert.centerPos = offsetCenter / screenScale;
            outVert.screenPos = pos / screenScale;
            outVert.midDir = centers[2].nDir * (isEnd ? 1 : -1);
        }
    } else {
        if (joinType == WKSVertexLineJoinNone || !intersectValid) {
            // Trivial case, just use the corner
            outVert.position = float4(pos, 0, 1);
            return outVert;
        }

        // Since there is one, use the intersect point by default
        pos = interPt;

        // We'll need the corner on the opposite side for several things.
        const float2 realOtherEdge = -interSgn * w2 * screenScale;
        const float2 otherCorner = offsetCenter + centers[2].norm * realOtherEdge;

        // For a bevel, use the intersect point for the inside of the turn, but not the outside.
        // Round piggypacks on bevel.
        if (joinType == WKSVertexLineJoinBevel || joinType == WKSVertexLineJoinRound) {
            switch (whichVert) {
                // Start cap, 0-3-1, 0-2-3
                case 2: case 10:
                    discardTri = true;  // Not using triangle #2
                    break;
                // Merge inside corners to avoid overlap, use default outside corner.
                case 0: case 3: case 8: case 9:
                case 4: case 5: case 6: case 7:
                    pos = isInsideEdge ? interPt : corner;
                    break;
                case 1: case 11: {
                    // Use the point halfway between the outside corner and the one on the opposite side.
                    const float2 norm = centers[(whichVert == 1) ? 1 : 3].norm;
                    const float2 c = offsetCenter + centers[2].norm * realEdge * turnSgn;
                    const float2 e = turnSgn * interSgn * w2 * screenScale;
                    pos = (offsetCenter + c + norm * e) / 2;
                    break;
                }
            }

            // For the round case, extend the center of the bevel out into a "tip," which will be
            // turned into a round extension by the fragment shader.  This isn't exactly right, but
            // I think we need more geometry to do better.
            // todo: fix texture Y-coords
            if (joinType == WKSVertexLineJoinRound && whichPoly != WideVecPolyBodyGeom && !discardTri) {
                outVert.roundJoin = true;
                outVert.centerPos = offsetCenter / screenScale;
                outVert.screenPos = pos / screenScale;

                // Direction bisecting the turn toward the outside (right for a left turn)
                const float2 mid = isEnd ? (centers[2].nDir - centers[3].nDir) :
                                           (centers[1].nDir - centers[2].nDir);
                outVert.midDir = normalize(mid / screenScale);

                if (whichVert == 1 || whichVert == 11) {
                    // Extend the corner far enough to cover the necessary round-ness.
                    // This should probably be related to the turn angle, we're just fudging it.
                    const float extend = 2 * w2;
                    pos += outVert.midDir * extend * screenScale;
                    outVert.screenPos = pos / screenScale;
                }
            }
        } else if (joinType == WKSVertexLineJoinMiterClip) {
            // Direction of intersect point (bisecting the segment directions)
            const float2 interVec = (realInterPt - offsetCenter) / screenScale;
            const float2 interDir = normalize(interVec) * turnSgn * interSgn;
            // And the perpendicular
            const float2 interNorm = float2(-interDir.y, interDir.x);
            // Distance from centerline intersect and edge intersect
            const float interDist = length(interVec);
            // "the miter is clipped by a line perpendicular to the line bisecting
            //  the angle between the two path segments at a distance of half the
            //  value of miter length from the intersection of the two path segments."
            const float midExt = miterLength / 2 * strokeWidth + wideVec.edge;

            switch (whichVert) {
                // Start cap, 0-3-1, 0-2-3
                case 0: case 8: {
                    // Out to the miter cap, then perpendicular to the line edge.
                    // The half-width of the clipped miter cap is the tangent of half the interior
                    // turn angle times the amount the original intersection extends beyond the cap.
                    const float miterEdgeW2 = (interDist - midExt) * tan(theta / 2);
                    pos = offsetCenter + screenScale * (interDir * midExt -
                           interNorm * miterEdgeW2 * turnSgn * (whichVert ? -1 : 1));
                    break;
                }
                case 1: case 10:
                    // Extend the turn bisector to the miter clip edge
                    pos = turningLeft ? (offsetCenter + interDir * midExt * screenScale) :
                                        ((whichVert == 1) ? corner : otherCorner);
                    break;
                case 2: case 9:
                    // Extend segment endpoint outward along the intersection angle by the miter length
                    pos = turningLeft ? ((whichVert == 2) ? corner : otherCorner) :
                                        (offsetCenter + interDir * midExt * screenScale);
                    break;
                case 3: case 11:
                    // The edge intersect, or the one on the other side
                    pos = turningLeft ? interPt : (offsetCenter + (offsetCenter - interPt));
                    break;
                // Body segment, 4-7-5, 4-6-7
                // Merge inside corners to avoid overlap, use default outside corner.
                case 4: case 5: case 6: case 7:
                    pos = isInsideEdge ? interPt : corner;
                    break;
            }
        }
    }

    if (!discardTri) {
        outVert.position = float4(pos, 0, 1);
    }

    return outVert;
}

// Fragment shader that takes the back of the globe into account
fragment float4 fragmentTri_wideVecPerf(
            ProjVertexTriWideVecPerf vert [[stage_in]],
            constant Uniforms &uniforms [[ buffer(idWideVectorUniformsUBO) ]])
{
    float patternAlpha = 1.0;

    // Reduce alpha of the "tip" for round joins to get a round look.
    float roundAlpha = 1.0;
    if (vert.roundJoin && dot(vert.screenPos - vert.centerPos, vert.midDir) > 0) {
        const float r = length_squared(vert.screenPos - vert.centerPos) / vert.w2 / vert.w2;
        roundAlpha = clamp((r > 0.95) ? (1 - r) * 20 : 1.0, 0.0, 1.0);
    }

    // Reduce alpha along the edges to get smooth blending.
    const float edgeAlpha = (vert.edge > 0) ? clamp((1 - abs(vert.texCoord.x)) * vert.w2 / vert.edge, 0.0, 1.0) : 1.0;

    return vert.color * float4(1,1,1,edgeAlpha * patternAlpha * roundAlpha);
}

)";
};

} // namespace shaders
} // namespace mbgl
