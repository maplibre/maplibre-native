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
#import "GLTFNodeVisitor.h"
#import "GLTFObject.h"
#import "GLTFUtilities.h"

NS_ASSUME_NONNULL_BEGIN

@class GLTFCamera, GLTFSkin, GLTFMesh;
@class GLTFKHRLight;

@interface GLTFNode : GLTFObject <GLTFNodeVisitable>
@property (nonatomic, weak) GLTFCamera *_Nullable camera;
@property (nonatomic, weak) GLTFKHRLight *_Nullable light;
@property (nonatomic, weak) GLTFNode *_Nullable parent;
@property (nonatomic, copy) NSArray<GLTFNode *> *children;
@property (nonatomic, weak) GLTFSkin *_Nullable skin;
@property (nonatomic, copy) NSString *_Nullable jointName;
@property (nonatomic, weak) GLTFMesh *_Nullable mesh;
@property (nonatomic, copy) NSArray<NSNumber *> *morphTargetWeights;
@property (nonatomic, assign) GLTFQuaternion rotationQuaternion;
@property (nonatomic, assign) simd_float3 scale;
@property (nonatomic, assign) simd_float3 translation;
@property (nonatomic, assign) simd_float4x4 localTransform;
@property (nonatomic, readonly, assign) simd_float4x4 globalTransform;
@property (nonatomic, readonly, assign)
    GLTFBoundingBox approximateBounds;  // axis-aligned; in local coordinates

- (void)addChildNode:(GLTFNode *)node;
- (void)removeFromParent;

@end

NS_ASSUME_NONNULL_END
