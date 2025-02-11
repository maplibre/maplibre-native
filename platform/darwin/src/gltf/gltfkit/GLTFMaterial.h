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

#import "GLTFObject.h"
#import "GLTFTexture.h"

#import <simd/simd.h>

NS_ASSUME_NONNULL_BEGIN

@class GLTFParameter;

typedef NS_ENUM(NSInteger, GLTFAlphaMode) {
  GLTFAlphaModeOpaque,
  GLTFAlphaModeMask,
  GLTFAlphaModeBlend,
};

@interface GLTFMaterial : GLTFObject

@property (nonatomic, assign) simd_float4 baseColorFactor;
@property (nonatomic, assign) float metalnessFactor;
@property (nonatomic, assign) float roughnessFactor;
@property (nonatomic, assign) float normalTextureScale;
@property (nonatomic, assign) float occlusionStrength;
@property (nonatomic, assign) simd_float3 emissiveFactor;

@property (nonatomic, assign)
    float glossinessFactor;  // Only used by KHR_materials_pbrSpecularGlossiness extension
@property (nonatomic, assign)
    simd_float3 specularFactor;  // Only used by KHR_materials_pbrSpecularGlossiness extension

@property (nonatomic, strong) GLTFTextureInfo* _Nullable baseColorTexture;
@property (nonatomic, strong) GLTFTextureInfo* _Nullable metallicRoughnessTexture;
@property (nonatomic, strong) GLTFTextureInfo* _Nullable normalTexture;
@property (nonatomic, strong) GLTFTextureInfo* _Nullable emissiveTexture;
@property (nonatomic, strong) GLTFTextureInfo* _Nullable occlusionTexture;

@property (nonatomic, assign)
    BOOL hasTextureTransforms;  // Only used when KHR_texture_transform extension is present

@property (nonatomic, assign, getter=isDoubleSided) BOOL doubleSided;

@property (nonatomic, assign) GLTFAlphaMode alphaMode;
@property (nonatomic, assign) float alphaCutoff;  // Only used when `alphaMode` == GLTFAlphaModeMask

@property (nonatomic, assign, getter=isUnlit)
    BOOL unlit;  // Only used when KHR_materials_unlit extension is present

@end

NS_ASSUME_NONNULL_END
