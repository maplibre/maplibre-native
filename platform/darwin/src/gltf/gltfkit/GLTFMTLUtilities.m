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

#import "GLTFMTLUtilities.h"

MTLPrimitiveType GLTFMTLPrimitiveTypeForPrimitiveType(GLTFPrimitiveType gltfType) {
    switch (gltfType) {
        case GLTFPrimitiveTypePoints:
            return MTLPrimitiveTypePoint;
        case GLTFPrimitiveTypeLines:
            return MTLPrimitiveTypeLine;
        case GLTFPrimitiveTypeLineStrip:
            return MTLPrimitiveTypeLineStrip;
        case GLTFPrimitiveTypeTriangles:
            return MTLPrimitiveTypeTriangle;
        case GLTFPrimitiveTypeTriangleStrip:
            return MTLPrimitiveTypeTriangleStrip;
            
            // Not supported; need to duplicate first element and restitch into tri strip, respectively
        case GLTFPrimitiveTypeLineLoop:
        case GLTFPrimitiveTypeTriangleFan:
        default:
            return -1;
    }
}

MTLBlendOperation GLTFMTLBlendOperationForBlendFunction(GLTFBlendFunction f) {
    switch (f) {
        case GLTFBlendFunctionAdd:
            return MTLBlendOperationAdd;
        case GLTFBlendFunctionSubtract:
            return MTLBlendOperationSubtract;
        case GLTFBlendFunctionReverseSubtract:
            return MTLBlendOperationReverseSubtract;
    }
}

MTLBlendFactor GLTFBlendFactorForBlendEquation(GLTFBlendEquation e) {
    switch (e) {
        case GLTFBlendEquationOne:
            return MTLBlendFactorOne;
        case GLTFBlendEquationZero:
            return MTLBlendFactorZero;
        case GLTFBlendEquationSrcAlpha:
            return MTLBlendFactorSourceAlpha;
        case GLTFBlendEquationSrcColor:
            return MTLBlendFactorSourceColor;
        case GLTFBlendEquationDestAlpha:
            return MTLBlendFactorDestinationAlpha;
        case GLTFBlendEquationDestColor:
            return MTLBlendFactorDestinationColor;
        case GLTFBlendEquationOneMinusSrcAlpha:
            return MTLBlendFactorOneMinusSourceAlpha;
        case GLTFBlendEquationOneMinusSrcColor:
            return MTLBlendFactorOneMinusSourceColor;
        case GLTFBlendEquationSrcAlphaSaturate:
            return MTLBlendFactorSourceAlphaSaturated;
        case GLTFBlendEquationOneMinusDestAlpha:
            return MTLBlendFactorOneMinusDestinationAlpha;
        case GLTFBlendEquationOneMinusDestColor:
            return MTLBlendFactorOneMinusDestinationColor;
        case GLTFBlendEquationOneMinusConstAlpha:
            return MTLBlendFactorOneMinusDestinationColor;
        default:
            NSLog(@"Unsupported blend equation %d", (int)e);
            return MTLBlendFactorOne;
    }
}

MTLCompareFunction GLTFMTLCompareFunctionForComparisonFunc(GLTFComparisonFunc f) {
    switch (f) {
        case GLTFComparisonFuncLess:
            return MTLCompareFunctionLess;
        case GLTFComparisonFuncEqual:
            return MTLCompareFunctionEqual;
        case GLTFComparisonFuncAlways:
            return MTLCompareFunctionAlways;
        case GLTFComparisonFuncGreater:
            return MTLCompareFunctionGreater;
        case GLTFComparisonFuncNotEqual:
            return MTLCompareFunctionNotEqual;
        case GLTFComparisonFuncLessEqual:
            return MTLCompareFunctionLessEqual;
        case GLTFComparisonFuncGreaterEqual:
            return MTLCompareFunctionGreaterEqual;
        default:
            NSLog(@"Unsupported comparison function %d", (int)f);
            return MTLCompareFunctionLess;
    }
}

MTLWinding GLTFMTLWindingForWinding(GLTFWinding w) {
    switch (w) {
        case GLTFWindingCounterclockwise:
            return MTLWindingCounterClockwise;
        case GLTFWindingClockwise:
        default:
            return MTLWindingClockwise;
    }
}

MTLCullMode GLTFMTLCullModeForCullFace(GLTFFace face) {
    switch (face) {
        case GLTFFaceBack:
            return MTLCullModeBack;
        case GLTFFaceFront:
            return MTLCullModeFront;
        default:
            return MTLCullModeBack;
    }
}

MTLSamplerMinMagFilter GLTFMTLSamplerMinMagFilterForSamplingFilter(GLTFSamplingFilter mode) {
    switch (mode) {
        case GLTFSamplingFilterNearest:
            return MTLSamplerMinMagFilterNearest;
        default:
            return MTLSamplerMinMagFilterLinear;
    }
}

MTLSamplerMipFilter GLTFMTLSamplerMipFilterForSamplingFilter(GLTFSamplingFilter mode) {
    switch (mode) {
        case GLTFSamplingFilterNearest:
        case GLTFSamplingFilterLinear:
            return MTLSamplerMipFilterNotMipmapped;
        case GLTFSamplingFilterNearestMipNearest:
        case GLTFSamplingFilterLinearMipNearest:
            return MTLSamplerMipFilterNearest;
        default:
            return MTLSamplerMipFilterLinear;
    }
}

MTLSamplerAddressMode GLTFMTLSamplerAddressModeForSamplerAddressMode(GLTFAddressMode mode) {
    switch (mode) {
        case GLTFAddressModeClampToEdge:
            return MTLSamplerAddressModeClampToEdge;
        case GLTFAddressModeMirroredRepeat:
            return MTLSamplerAddressModeMirrorRepeat;
        default:
            return MTLSamplerAddressModeRepeat;
    }
}

NSString *GLTFMTLTypeNameForType(GLTFDataType baseType, GLTFDataDimension dimension, BOOL packed) {
    NSString *typeName = @"float";
    NSString *packingPrefix = @"";
    NSString *dimensionSuffix = @"";
    
    if (packed && (dimension != GLTFDataDimensionScalar)) {
        packingPrefix = @"packed_";
    }

    switch (baseType) {
        case GLTFDataTypeBool:      typeName = @"bool";      break;
        case GLTFDataTypeChar:      typeName = @"char";      break;
        case GLTFDataTypeUChar:     typeName = @"uchar";     break;
        case GLTFDataTypeShort:     typeName = @"short";     break;
        case GLTFDataTypeUShort:    typeName = @"ushort";    break;
        case GLTFDataTypeInt:       typeName = @"int";       break;
        case GLTFDataTypeUInt:      typeName = @"uint";      break;
        case GLTFDataTypeFloat:     typeName = @"float";     break;
        case GLTFDataTypeSampler2D: typeName = @"texture2d"; break;
        default:
            return @"__UNKNOWN_TYPE__";
    }
    
    switch (dimension) {
        case GLTFDataDimensionScalar:    dimensionSuffix = @"";         break;
        case GLTFDataDimensionVector2:   dimensionSuffix = @"2";        break;
        case GLTFDataDimensionVector3:   dimensionSuffix = @"3";        break;
        case GLTFDataDimensionVector4:   dimensionSuffix = @"4";        break;
        case GLTFDataDimensionMatrix2x2: dimensionSuffix = @"float2x2"; break;
        case GLTFDataDimensionMatrix3x3: dimensionSuffix = @"float3x3"; break;
        case GLTFDataDimensionMatrix4x4: dimensionSuffix = @"float4x4"; break;
        default:
            return @"__UNKNOWN_TYPE__";
    }
    
    return [NSString stringWithFormat:@"%@%@%@", packingPrefix, typeName, dimensionSuffix];
}

MTLVertexFormat GLTFMTLVertexFormatForComponentTypeAndDimension(GLTFDataType baseType, GLTFDataDimension dimension)
{
    switch (baseType) {
        case GLTFDataTypeChar:
            switch (dimension) {
                case GLTFDataDimensionVector2:
                    return MTLVertexFormatChar2;
                case GLTFDataDimensionVector3:
                    return MTLVertexFormatChar3;
                case GLTFDataDimensionVector4:
                    return MTLVertexFormatChar4;
                default:
                    break;
            }
        case GLTFDataTypeUChar:
            switch (dimension) {
                case GLTFDataDimensionVector2:
                    return MTLVertexFormatUChar2;
                case GLTFDataDimensionVector3:
                    return MTLVertexFormatUChar3;
                case GLTFDataDimensionVector4:
                    return MTLVertexFormatUChar4;
                default:
                    break;
            }
        case GLTFDataTypeShort:
            switch (dimension) {
                case GLTFDataDimensionVector2:
                    return MTLVertexFormatShort2;
                case GLTFDataDimensionVector3:
                    return MTLVertexFormatShort3;
                case GLTFDataDimensionVector4:
                    return MTLVertexFormatShort4;
                default:
                    break;
            }
        case GLTFDataTypeUShort:
            switch (dimension) {
                case GLTFDataDimensionVector2:
                    return MTLVertexFormatUShort2;
                case GLTFDataDimensionVector3:
                    return MTLVertexFormatUShort3;
                case GLTFDataDimensionVector4:
                    return MTLVertexFormatUShort4;
                default:
                    break;
            }
        case GLTFDataTypeInt:
            switch (dimension) {
                case GLTFDataDimensionScalar:
                    return MTLVertexFormatInt;
                case GLTFDataDimensionVector2:
                    return MTLVertexFormatInt2;
                case GLTFDataDimensionVector3:
                    return MTLVertexFormatInt3;
                case GLTFDataDimensionVector4:
                    return MTLVertexFormatInt4;
                default:
                    break;
            }
        case GLTFDataTypeUInt:
            switch (dimension) {
                case GLTFDataDimensionScalar:
                    return MTLVertexFormatUInt;
                case GLTFDataDimensionVector2:
                    return MTLVertexFormatUInt2;
                case GLTFDataDimensionVector3:
                    return MTLVertexFormatUInt3;
                case GLTFDataDimensionVector4:
                    return MTLVertexFormatUInt4;
                default:
                    break;
            }
        case GLTFDataTypeFloat:
            switch (dimension) {
                case GLTFDataDimensionScalar:
                    return MTLVertexFormatFloat;
                case GLTFDataDimensionVector2:
                    return MTLVertexFormatFloat2;
                case GLTFDataDimensionVector3:
                    return MTLVertexFormatFloat3;
                case GLTFDataDimensionVector4:
                    return MTLVertexFormatFloat4;
                default:
                    break;
            }
        default:
            break;
    }
    
    return MTLVertexFormatInvalid;
}


