/*
 *  DefaultShadersMTL.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
 *  Copyright 2011-2022 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

namespace WhirlyKitShader
{

/** Expressions are used to change values like width and opacity over zoom levels. **/
#define WKSExpStops 8

/** Attributes within the [[stage_in]] for vertex shaders **/
    
// Basic vertex attribute positions
typedef enum {
    WKSVertexPositionAttribute = 0,
    WKSVertexColorAttribute = 1,
    WKSVertexNormalAttribute = 2,
    WKSVertexTextureBaseAttribute = 3,
    // Need some space for textures
    WKSVertexMaskAttribute = 5
    // And another space for extra mask
} WKSVertexAttributes;
    
// Wide Vector vertex attribute positions
typedef enum {
    WKSVertexWideVecTexInfoAttribute = 7,
    // We don't use these at the same time
    WKSVertexWideVecInstIndexAttribute = 7,
    WKSVertexWideVecP1Attribute,
    WKSVertexWideVecN0Attribute,
    WKSVertexWideVecC0Attribute,
    WKSVertexWideVecOffsetAttribute
} WKSVertexWideVecAttributes;
    
// Line Joins
// These are assumed to match WideVectorLineJoinType
typedef enum {
    WKSVertexLineJoinMiter       = 0,
    WKSVertexLineJoinMiterClip   = 1,
    WKSVertexLineJoinMiterSimple = 2,
    WKSVertexLineJoinRound       = 3,
    WKSVertexLineJoinBevel       = 4,
    WKSVertexLineJoinNone        = 5,
} WKSVertexLineJoinType;

// Line Caps
// These are assumed to match WideVectorLineCapType
typedef enum {
    WKSVertexLineCapButt = 0,
    WKSVertexLineCapRound = 1,
    WKSVertexLineCapSquare = 2,
} WKSVertexLineCapType;

// Maximum number of textures we currently support
#define WKSTextureMax 8
// Textures passed into the shader start here
#define WKSTextureEntryLookup 5

// All the buffer entries (other than stage_in) for the vertex shaders
typedef enum {
    WKSVertexBuffer = 12,
    WKSVertUniformArgBuffer,
    WKSVertLightingArgBuffer,
    // These are free form with their own subsections
    WKSVertexArgBuffer,
    // Textures are optional
    WKSVertTextureArgBuffer,
    // Model instances
    WKSVertModelInstanceArgBuffer,
    WKSVertCalculationArgBuffer, // Leave some room for more than one of these
    // If we're using the indirect instancing (can be driven by the GPU) this is
    //  where the indirect buffer lives
    WKSVertInstanceIndirectBuffer = WKSVertCalculationArgBuffer + 4,
    WKSVertMaxBuffer
} WKSVertexArgumentBuffers;

// All the buffer entries for the fragment shaders
typedef enum {
    WKSFragUniformArgBuffer = 0,
    WKSFragLightingArgBuffer = 1,
    WKSFragmentArgBuffer = 2,
    WKSFragTextureArgBuffer = 4,
    WKSFragMaxBuffer
} WKSFragArgumentBuffer;

// Entries in the free form argument buffer
// These must be in order, but you can add new ones at the end
typedef enum {
    WKSUniformDrawStateEntry = 0,
    WKSUniformVecEntryExp = 99,
    WKSUniformWideVecEntry = 100,
    WKSUniformWideVecEntryExp = 110,
    WKSUniformScreenSpaceEntry = 200,
    WKSUniformScreenSpaceEntryExp = 210,
    WKSUniformModelInstanceEntry = 300,
    WKSUniformBillboardEntry = 400,
    WKSUniformParticleStateEntry = 410
} WKSArgBufferEntries;

// Uniforms for the basic case.  Nothing fancy.
struct Uniforms
{
    // TODO: Diff matrixes can be 0s
    simd::float4x4 mvpMatrix;
    simd::float4x4 mvpMatrixDiff;
    simd::float4x4 mvMatrix;
    simd::float4x4 mvMatrixDiff;
    simd::float4x4 pMatrix;
    simd::float4x4 pMatrixDiff;
    simd::float2 frameSize;    // Output framebuffer size   // FILL
};

// Things that change per drawable (like fade)
struct UniformDrawStateA {
    simd::float4x4 singleMat; // Individual transform used by model instances
    simd::float2 screenOrigin; // Used for texture pinning in screen space
    float interp;              // Used to interpolate between two textures (if appropriate)
    int outputTexLevel;        // Normally 0, unless we're running a reduce
    int whichOffsetMatrix;     // Normally 0, unless we're in 2D mode drawing the same stuff multiple times
    float fadeUp,fadeDown;     // Fading in/out values
    float minVisible,maxVisible;  // Visibility by height
    float minVisibleFadeBand,maxVisibleFadeBand;
    int zoomSlot;              // Used to pass continuous zoom info
    bool clipCoords;           // If set, the geometry coordinates aren't meant to be transformed
    bool hasExp;               // Look for a UniformWideVecExp structure for color, opacity, and width
};

// Instructions to the wide vector shaders, usually per-drawable
struct UniformWideVec {
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
    // Color for the whole line
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
    float3 screenPos [[attribute(WhirlyKitShader::WKSVertexPositionAttribute)]];
    float4 color [[attribute(WhirlyKitShader::WKSVertexColorAttribute)]];
    int index [[attribute(WhirlyKitShader::WKSVertexWideVecInstIndexAttribute)]];
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
