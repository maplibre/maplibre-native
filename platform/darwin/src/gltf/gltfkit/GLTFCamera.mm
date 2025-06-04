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

#import "GLTFCamera.h"

@interface GLTFCamera ()
@property (nonatomic, assign, getter=projectionMatrixIsDirty) BOOL projectionMatrixDirty;
@end

@implementation GLTFCamera

@synthesize projectionMatrix=_projectionMatrix;

- (instancetype)init {
    if ((self = [super init])) {
        _referencingNodes = @[];
    }
    return self;
}

- (void)setCameraType:(GLTFCameraType)cameraType {
    _cameraType = cameraType;
    _projectionMatrixDirty = YES;
}

- (void)setAspectRatio:(float)aspectRatio {
    _aspectRatio = aspectRatio;
    _projectionMatrixDirty = YES;
}

- (void)setYfov:(float)yfov {
    _yfov = yfov;
    _projectionMatrixDirty = YES;
}

- (void)setXmag:(float)xmag {
    _xmag = xmag;
    _projectionMatrixDirty = YES;
}

- (void)setYmag:(float)ymag {
    _ymag = ymag;
    _projectionMatrixDirty = YES;
}

- (void)setZnear:(float)znear {
    _znear = znear;
    _projectionMatrixDirty = YES;
}

- (void)setZfar:(float)zfar {
    _zfar = zfar;
    _projectionMatrixDirty = YES;
}

- (void)setProjectionMatrix:(simd_float4x4)projectionMatrix {
    _projectionMatrix = projectionMatrix;
    _projectionMatrixDirty = NO;
}

- (simd_float4x4)projectionMatrix {
    if (self.projectionMatrixIsDirty) {
        [self _buildProjectionMatrix];
    }
    return _projectionMatrix;
}

- (void)_buildProjectionMatrix {
    switch (_cameraType) {
        case GLTFCameraTypeOrthographic: {
            simd_float4 X = (simd_float4){ 1 / _xmag, 0, 0, 0 };
            simd_float4 Y = (simd_float4){ 0, 1 / _ymag, 0, 0 };
            simd_float4 Z = (simd_float4){ 0, 0, 2 / (_znear - _zfar), 0 };
            simd_float4 W = (simd_float4){ 0, 0, (_zfar + _znear) / (_znear - _zfar), 1 };
            _projectionMatrix = (simd_float4x4){ { X, Y, Z, W } };
            break;
        }
        case GLTFCameraTypePerspective:
        default: {
            simd_float4 X = (simd_float4){ 1 / (_aspectRatio * tanf(0.5 * _yfov)), 0, 0, 0 };
            simd_float4 Y = (simd_float4){ 0, 1 / tanf(0.5 * _yfov), 0, 0 };
            simd_float4 Z = (simd_float4){ 0, 0, -1, -1 };
            simd_float4 W = (simd_float4){ 0, 0, -2 * _znear, 0 };
            if (_zfar != FLT_MAX) {
                Z = (simd_float4){ 0, 0, (_zfar + _znear) / (_znear - _zfar), -1 };
                W = (simd_float4){ 0, 0, (2 * _zfar * _znear) / (_znear - _zfar), 0 };
            }
            _projectionMatrix = (simd_float4x4){ { X, Y, Z, W } };
            break;
        }
    }
    simd_float4x4 glToMetal = (simd_float4x4){{
        { 1, 0,   0, 0 },
        { 0, 1,   0, 0 },
        { 0, 0, 0.5, 0 },
        { 0, 0, 0.5, 1 },
    }};
    _projectionMatrix = simd_mul(glToMetal, _projectionMatrix);
    _projectionMatrixDirty = NO;
}

@end
