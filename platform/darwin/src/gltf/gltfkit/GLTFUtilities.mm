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

#import "GLTFUtilities.h"
#include "GLTFEnums.h"

bool GLTFBoundingBoxIsEmpty(GLTFBoundingBox b) {
    return (b.minPoint.x == b.maxPoint.x) && (b.minPoint.y == b.maxPoint.y) && (b.minPoint.z == b.maxPoint.z);
}

GLTFBoundingBox *GLTFBoundingBoxUnion(GLTFBoundingBox *a, GLTFBoundingBox b) {
    bool leftEmpty = GLTFBoundingBoxIsEmpty(*a);
    bool rightEmpty = GLTFBoundingBoxIsEmpty(b);
    
    if (leftEmpty) {
        if (!rightEmpty) {
            *a = b;
        }
    } else if (!rightEmpty) {
        if (b.minPoint.x < a->minPoint.x) { a->minPoint.x = b.minPoint.x; }
        if (b.minPoint.y < a->minPoint.y) { a->minPoint.y = b.minPoint.y; }
        if (b.minPoint.z < a->minPoint.z) { a->minPoint.z = b.minPoint.z; }
        if (b.maxPoint.x > a->maxPoint.x) { a->maxPoint.x = b.maxPoint.x; }
        if (b.maxPoint.y > a->maxPoint.y) { a->maxPoint.y = b.maxPoint.y; }
        if (b.maxPoint.z > a->maxPoint.z) { a->maxPoint.z = b.maxPoint.z; }
    }
    
    return a;
}

void GLTFBoundingBoxTransform(GLTFBoundingBox *b, simd_float4x4 transform) {
    simd_float4 ltf = (simd_float4) { b->minPoint.x, b->maxPoint.y, b->maxPoint.z, 1 };
    simd_float4 rtf = (simd_float4) { b->maxPoint.x, b->maxPoint.y, b->maxPoint.z, 1 };
    simd_float4 lbf = (simd_float4) { b->minPoint.x, b->minPoint.y, b->maxPoint.z, 1 };
    simd_float4 rbf = (simd_float4) { b->maxPoint.x, b->minPoint.y, b->maxPoint.z, 1 };
    simd_float4 ltb = (simd_float4) { b->minPoint.x, b->maxPoint.y, b->minPoint.z, 1 };
    simd_float4 rtb = (simd_float4) { b->maxPoint.x, b->maxPoint.y, b->minPoint.z, 1 };
    simd_float4 lbb = (simd_float4) { b->minPoint.x, b->minPoint.y, b->minPoint.z, 1 };
    simd_float4 rbb = (simd_float4) { b->maxPoint.x, b->minPoint.y, b->minPoint.z, 1 };
    
    ltf = matrix_multiply(transform, ltf);
    rtf = matrix_multiply(transform, rtf);
    lbf = matrix_multiply(transform, lbf);
    rbf = matrix_multiply(transform, rbf);
    ltb = matrix_multiply(transform, ltb);
    rtb = matrix_multiply(transform, rtb);
    lbb = matrix_multiply(transform, lbb);
    rbb = matrix_multiply(transform, rbb);
    
    b->minPoint.x = fmin(fmin(fmin(fmin(fmin(fmin(fmin(ltf.x, rtf.x), lbf.x), rbf.x), ltb.x), rtb.x), lbb.x), rbb.x);
    b->minPoint.y = fmin(fmin(fmin(fmin(fmin(fmin(fmin(ltf.y, rtf.y), lbf.y), rbf.y), ltb.y), rtb.y), lbb.y), rbb.y);
    b->minPoint.z = fmin(fmin(fmin(fmin(fmin(fmin(fmin(ltf.z, rtf.z), lbf.z), rbf.z), ltb.z), rtb.z), lbb.z), rbb.z);
    b->maxPoint.x = fmax(fmax(fmax(fmax(fmax(fmax(fmax(ltf.x, rtf.x), lbf.x), rbf.x), ltb.x), rtb.x), lbb.x), rbb.x);
    b->maxPoint.y = fmax(fmax(fmax(fmax(fmax(fmax(fmax(ltf.y, rtf.y), lbf.y), rbf.y), ltb.y), rtb.y), lbb.y), rbb.y);
    b->maxPoint.z = fmax(fmax(fmax(fmax(fmax(fmax(fmax(ltf.z, rtf.z), lbf.z), rbf.z), ltb.z), rtb.z), lbb.z), rbb.z);
}



GLTFQuaternion GLTFQuaternionFromEulerAngles(float pitch, float yaw, float roll) {
    float cx = cos(pitch / 2);
    float sx = sin(pitch / 2);
    float cy = cos(yaw / 2);
    float sy = sin(yaw / 2);
    float cz = cos(roll / 2);
    float sz = sin(roll / 2);
    
    GLTFQuaternion q = simd_quaternion(
        sx*cy*cz + cx*sy*sz,
        cx*sy*cz + sx*cy*sz,
        cx*cy*sz - sx*sy*cz,
        cx*cy*cz - sx*sy*sz
    );
    return q;
}











GLTFDataDimension GLTFDataDimensionForName(NSString *name) {
    if ([name isEqualToString:@"SCALAR"]) {
        return GLTFDataDimensionScalar;
    } else if ([name isEqualToString:@"VEC2"]) {
        return GLTFDataDimensionVector2;
    } else if ([name isEqualToString:@"VEC2"]) {
        return GLTFDataDimensionVector2;
    } else if ([name isEqualToString:@"VEC3"]) {
        return GLTFDataDimensionVector3;
    } else if ([name isEqualToString:@"VEC4"]) {
        return GLTFDataDimensionVector4;
    } else if ([name isEqualToString:@"MAT2"]) {
        return GLTFDataDimensionMatrix2x2;
    } else if ([name isEqualToString:@"MAT3"]) {
        return GLTFDataDimensionMatrix3x3;
    } else if ([name isEqualToString:@"MAT4"]) {
        return GLTFDataDimensionMatrix4x4;
    }
    
    return GLTFDataDimensionUnknown;
}

size_t GLTFSizeOfDataType(GLTFDataType type) {
    
    switch (type) {
        case GLTFDataTypeChar:      return sizeof(char);
        case GLTFDataTypeUChar:     return sizeof(unsigned char);
        case GLTFDataTypeShort:     return sizeof(short);
        case GLTFDataTypeUShort:    return sizeof(unsigned short);
        case GLTFDataTypeInt:       return sizeof(int);
        case GLTFDataTypeUInt:      return sizeof(unsigned int);
        case GLTFDataTypeFloat:     return sizeof(float);
        case GLTFDataTypeFloat2:    return sizeof(float) * 2;
        case GLTFDataTypeFloat3:    return sizeof(float) * 3;
        case GLTFDataTypeFloat4:    return sizeof(float) * 4;
        case GLTFDataTypeInt2:      return sizeof(int)   * 2;
        case GLTFDataTypeInt3:      return sizeof(int)   * 3;
        case GLTFDataTypeInt4:      return sizeof(int)   * 4;
        case GLTFDataTypeBool:      return sizeof(bool);
        case GLTFDataTypeBool2:     return sizeof(bool)  * 2;
        case GLTFDataTypeBool3:     return sizeof(bool)  * 3;
        case GLTFDataTypeBool4:     return sizeof(bool)  * 4;
        case GLTFDataTypeFloat2x2:  return sizeof(float) * 4;
        case GLTFDataTypeFloat3x3:  return sizeof(float) * 9;
        case GLTFDataTypeFloat4x4:  return sizeof(float) * 16;
        case GLTFDataTypeSampler2D: return sizeof(size_t);
        default:                    return 0;
    }
}

size_t GLTFSizeOfComponentTypeWithDimension(GLTFDataType baseType, GLTFDataDimension dimension)
{
    switch (baseType) {
        case GLTFDataTypeChar:
        case GLTFDataTypeUChar:
            switch (dimension) {
                case GLTFDataDimensionVector2:
                    return 2;
                case GLTFDataDimensionVector3:
                    return 3;
                case GLTFDataDimensionVector4:
                    return 4;
                default:
                    break;
            }
        case GLTFDataTypeShort:
        case GLTFDataTypeUShort:
            switch (dimension) {
                case GLTFDataDimensionVector2:
                    return 4;
                case GLTFDataDimensionVector3:
                    return 6;
                case GLTFDataDimensionVector4:
                    return 8;
                default:
                    break;
            }
        case GLTFDataTypeInt:
        case GLTFDataTypeUInt:
        case GLTFDataTypeFloat:
            switch (dimension) {
                case GLTFDataDimensionScalar:
                    return 4;
                case GLTFDataDimensionVector2:
                    return 8;
                case GLTFDataDimensionVector3:
                    return 12;
                case GLTFDataDimensionVector4:
                case GLTFDataDimensionMatrix2x2:
                    return 16;
                case GLTFDataDimensionMatrix3x3:
                    return 36;
                case GLTFDataDimensionMatrix4x4:
                    return 64;
                default:
                    break;
            }
        default:
            break;
    }
    return 0;
}

NSInteger GLTFComponentCountForDimension(GLTFDataDimension dimension) {
    switch (dimension) {
        case GLTFDataDimensionScalar:
            return 1;
        case GLTFDataDimensionVector2:
            return 2;
        case GLTFDataDimensionVector3:
            return 3;
        case GLTFDataDimensionVector4:
            return 4;
        case GLTFDataDimensionMatrix2x2:
            return 4;
        case GLTFDataDimensionMatrix3x3:
            return 9;
        case GLTFDataDimensionMatrix4x4:
            return 16;
        default:
            return 0;
    }
}

BOOL GLTFDataTypeComponentsAreFloats(GLTFDataType type) {
    switch (type) {
        case GLTFDataTypeFloat:
        case GLTFDataTypeFloat2:
        case GLTFDataTypeFloat3:
        case GLTFDataTypeFloat4:
        case GLTFDataTypeFloat2x2:
        case GLTFDataTypeFloat3x3:
        case GLTFDataTypeFloat4x4:
            return YES;
        default:
            return NO;
    }
}

simd_float2 GLTFVectorFloat2FromArray(NSArray *array) {
    return (simd_float2){ [array[0] floatValue], [array[1] floatValue] };
}

simd_float3 GLTFVectorFloat3FromArray(NSArray *array) {
    return (simd_float3){ [array[0] floatValue], [array[1] floatValue], [array[2] floatValue] };
}

simd_float4 GLTFVectorFloat4FromArray(NSArray *array) {
    return (simd_float4){ [array[0] floatValue], [array[1] floatValue], [array[2] floatValue], [array[3] floatValue] };
}

GLTFQuaternion GLTFQuaternionFromArray(NSArray *array) {
    return simd_quaternion([array[0] floatValue], [array[1] floatValue], [array[2] floatValue], [array[3] floatValue]);
}

simd_float4x4 GLTFMatrixFloat4x4FromArray(NSArray *array) {
    return (simd_float4x4){ {
        {  [array[0] floatValue],  [array[1] floatValue],  [array[2] floatValue],  [array[3] floatValue] },
        {  [array[4] floatValue],  [array[5] floatValue],  [array[6] floatValue],  [array[7] floatValue] },
        {  [array[8] floatValue],  [array[9] floatValue], [array[10] floatValue], [array[11] floatValue] },
        { [array[12] floatValue], [array[13] floatValue], [array[14] floatValue], [array[15] floatValue] }
    } };
}
