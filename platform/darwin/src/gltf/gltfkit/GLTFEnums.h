//
//  Copyright (c) 2018 Warren Moore. All rights reserved.
//
//  Permission to use, copy, modify, and distribute this software for any
//  purpose with or without fee is hereby granted, provided that the above
//  copyright notice and this permission notice appear in all copies.
//
//  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
//  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
//  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
//  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
//  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
//  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
//  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSInteger, GLTFDataType) {
    GLTFBaseTypeUnknown,
    GLTFDataTypeChar      = 0x1400,
    GLTFDataTypeUChar     = 0x1401,
    GLTFDataTypeShort     = 0x1402,
    GLTFDataTypeUShort    = 0x1403,
    GLTFDataTypeInt       = 0x1404,
    GLTFDataTypeUInt      = 0x1405,
    GLTFDataTypeFloat     = 0x1406,
    
    GLTFDataTypeFloat2    = 0x8B50,
    GLTFDataTypeFloat3    = 0x8B51,
    GLTFDataTypeFloat4    = 0x8B52,
    GLTFDataTypeInt2      = 0x8B53,
    GLTFDataTypeInt3      = 0x8B54,
    GLTFDataTypeInt4      = 0x8B55,
    GLTFDataTypeBool      = 0x8B56,
    GLTFDataTypeBool2     = 0x8B57,
    GLTFDataTypeBool3     = 0x8B58,
    GLTFDataTypeBool4     = 0x8B59,
    GLTFDataTypeFloat2x2  = 0x8B5A,
    GLTFDataTypeFloat3x3  = 0x8B5B,
    GLTFDataTypeFloat4x4  = 0x8B5C,
    GLTFDataTypeSampler2D = 0x8B5E,
};

typedef NS_ENUM(NSInteger, GLTFDataDimension) {
    GLTFDataDimensionUnknown,
    GLTFDataDimensionScalar,
    GLTFDataDimensionVector2,
    GLTFDataDimensionVector3,
    GLTFDataDimensionVector4,
    GLTFDataDimensionMatrix2x2,
    GLTFDataDimensionMatrix3x3,
    GLTFDataDimensionMatrix4x4,
};

typedef NS_ENUM(NSInteger, GLTFTarget) {
    GLTFTargetUnknown,
    GLTFTargetArrayBuffer        = 0x8892,
    GLTFTargetElementArrayBuffer = 0x8893,
};

typedef NS_ENUM(NSInteger, GLTFPrimitiveType) {
    GLTFPrimitiveTypePoints,
    GLTFPrimitiveTypeLines,
    GLTFPrimitiveTypeLineLoop,
    GLTFPrimitiveTypeLineStrip,
    GLTFPrimitiveTypeTriangles,
    GLTFPrimitiveTypeTriangleStrip,
    GLTFPrimitiveTypeTriangleFan,
};

typedef NS_ENUM(NSInteger, GLTFCameraType) {
    GLTFCameraTypePerspective,
    GLTFCameraTypeOrthographic,
};

typedef NS_ENUM(NSInteger, GLTFTextureTarget) {
    GLTFTextureTargetTexture2D = 0x0DE1,
};

typedef NS_ENUM(NSInteger, GLTFTextureFormat) {
    GLTFTextureFormatUnknown,
    GLTFTextureFormatAlpha          = 0x1906,
    GLTFTextureFormatRGB            = 0x1907,
    GLTFTextureFormatRGBA           = 0x1908,
    GLTFTextureFormatLuminance      = 0x1909,
    GLTFTextureFormatLuminanceAlpha = 0x190A,
};

typedef NS_ENUM(NSInteger, GLTFTextureType) {
    GLTFTextureTypeUnknown,
    GLTFTextureTypeUChar      = 0x1401,
    GLTFTextureTypeUShort565  = 0x8363,
    GLTFTextureTypeUShort4444 = 0x8033,
    GLTFTextureTypeUShort5551 = 0x8034,
};

typedef NS_ENUM(NSInteger, GLTFShaderType) {
    GLTFShaderTypeVertex   = 0x8B31,
    GLTFShaderTypeFragment = 0x8B30,
};

typedef NS_ENUM(NSInteger, GLTFSamplingFilter) {
    GLTFSamplingFilterUnknown,
    GLTFSamplingFilterNearest           = 0x2600,
    GLTFSamplingFilterLinear            = 0x2601,
    GLTFSamplingFilterNearestMipNearest = 0x2700,
    GLTFSamplingFilterLinearMipNearest  = 0x2701,
    GLTFSamplingFilterNearestMipLinear  = 0x2702,
    GLTFSamplingLinearMipLinear         = 0x2703,
};

typedef NS_ENUM(NSInteger, GLTFAddressMode) {
    GLTFAddressModeUnknown,
    GLTFAddressModeClampToEdge    = 0x812F,
    GLTFAddressModeMirroredRepeat = 0x8370,
    GLTFAddressModeRepeat         = 0x2901,
};

typedef NS_ENUM(NSInteger, GLTFComparisonFunc) {
    GLTFComparisonFuncLess         = 0x0201,
    GLTFComparisonFuncEqual        = 0x0202,
    GLTFComparisonFuncLessEqual    = 0x0203,
    GLTFComparisonFuncGreater      = 0x0204,
    GLTFComparisonFuncNotEqual     = 0x0205,
    GLTFComparisonFuncGreaterEqual = 0x0206,
    GLTFComparisonFuncAlways       = 0x0207,
};

typedef NS_ENUM(NSInteger, GLTFFace) {
    GLTFFaceFront        = 0x0404,
    GLTFFaceBack         = 0x405,
    GLTFFaceFrontAndBack = 0x408,
};

typedef NS_ENUM(NSInteger, GLTFWinding) {
    GLTFWindingClockwise        = 0x900,
    GLTFWindingCounterclockwise = 0x0901,
};

typedef NS_ENUM(NSInteger, GLTFState) {
    GLTFStateBlendingEnabled          = 0x0BE2,
    GLTFStateCullFaceEnabled          = 0x0B44,
    GLTFStateDepthTestEnabled         = 0x0B71,
    GLTFStatePolygonOffsetFillEnabled = 0x8037,
    GLTFStateAlphaToCoverageEnabled   = 0x809E,
    GLTFStateScissorTestEnabled       = 0x0C11,
};

typedef NS_ENUM(NSInteger, GLTFBlendFunction) {
    GLTFBlendFunctionAdd             = 0x8006,
    GLTFBlendFunctionSubtract        = 0x800A,
    GLTFBlendFunctionReverseSubtract = 0x800B,
};

typedef NS_ENUM(NSInteger, GLTFBlendEquation) {
    GLTFBlendEquationZero               = 0x0000,
    GLTFBlendEquationOne                = 0x0001,
    GLTFBlendEquationSrcColor           = 0x0300,
    GLTFBlendEquationOneMinusSrcColor   = 0x0301,
    GLTFBlendEquationSrcAlpha           = 0x0302,
    GLTFBlendEquationOneMinusSrcAlpha   = 0x0303,
    GLTFBlendEquationDestAlpha          = 0x0304,
    GLTFBlendEquationOneMinusDestAlpha  = 0x0305,
    GLTFBlendEquationDestColor          = 0x0306,
    GLTFBlendEquationOneMinusDestColor  = 0x0307,
    GLTFBlendEquationSrcAlphaSaturate   = 0x0308,
    GLTFBlendEquationConstantColor      = 0x8001,
    GLTFBlendEquationOneMinusConstColor = 0x8002,
    GLTFBlendEquationConstantAlpha      = 0x8003,
    GLTFBlendEquationOneMinusConstAlpha = 0x8004,
};

typedef NS_ENUM(NSInteger, GLTFInterpolationMode) {
    GLTFInterpolationModeStep,
    GLTFInterpolationModeLinear,
    GLTFInterpolationModeCubic,
};
