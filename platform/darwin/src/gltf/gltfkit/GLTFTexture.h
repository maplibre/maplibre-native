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

#import <simd/simd.h>
#import "../GLTFMath.hpp"
#import "GLTFEnums.h"
#import "GLTFObject.h"

NS_ASSUME_NONNULL_BEGIN

@class GLTFTextureSampler, GLTFImage;

@interface GLTFTexture : GLTFObject

@property (nonatomic, weak) GLTFTextureSampler *sampler;

@property (nonatomic, weak) GLTFImage *image;

// GLTFTextureFormatRGBA
@property (nonatomic, assign) GLTFTextureFormat format;

// GLTFTextureFormatRGBA
@property (nonatomic, assign) GLTFTextureFormat internalFormat;

// GLTFTextureTypeUChar
@property (nonatomic, assign) GLTFTextureType type;

// GLTFTextureTargetTexture2D
@property (nonatomic, assign) GLTFTextureTarget target;

@end

@interface GLTFTextureInfo : NSObject

@property (nonatomic, strong) GLTFTexture *texture;

@property (nonatomic, assign) NSInteger texCoord;

// The transform to apply to texture coordinates before sampling from this texture.
// Defaults to the identity transform. Only populated if KHR_texture_transform is included
// as an optional or required extension for the containing asset.
@property (nonatomic, assign) GLTFTextureTransform transform;

@property (nonatomic, strong) NSDictionary *_Nullable extensions;

@property (nonatomic, strong) NSDictionary *_Nullable extras;

@end

NS_ASSUME_NONNULL_END
