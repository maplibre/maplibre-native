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

#import "GLTFNode.h"
#import "GLTFAccessor.h"
#import "GLTFMesh.h"
#import "GLTFVertexDescriptor.h"

@interface GLTFNode ()
@property (nonatomic, assign, getter=localTransformIsDirty) BOOL localTransformDirty;
@property (nonatomic, strong) NSMutableArray *mutableChildren;
@end

@implementation GLTFNode

@synthesize localTransform=_localTransform;

- (instancetype)init {
    if ((self = [super init])) {
        _localTransform = matrix_identity_float4x4;
        _rotationQuaternion = simd_quaternion(0.f, 0.f, 0.f, 1.f);
        _scale = vector3(1.0f, 1.0f, 1.0f);
        _translation = vector3(0.0f, 0.0f, 0.0f);
        _morphTargetWeights = @[];
    }
    return self;
}

- (void)setChildren:(NSArray<GLTFNode *> *)children {
    _mutableChildren = [children mutableCopy];
}

- (NSArray<GLTFNode *> *)children {
    return [_mutableChildren copy];
}

- (void)addChildNode:(GLTFNode *)node {
    if (node.parent) {
        [node removeFromParent];
    }
    node.parent = self;
    [self.mutableChildren addObject:node];
}

- (void)removeFromParent {
    [self.parent _removeChildNode:self];
}

- (void)_removeChildNode:(GLTFNode *)child {
    [self.mutableChildren removeObject:child];
    child.parent = nil;
}

- (void)setScale:(simd_float3)scale {
    _scale = scale;
    _localTransformDirty = YES;
}

- (void)setRotationQuaternion:(GLTFQuaternion)rotationQuaternion {
    _rotationQuaternion = rotationQuaternion;
    _localTransformDirty = YES;
}

- (void)setTranslation:(simd_float3)translation {
    _translation = translation;
    _localTransformDirty = YES;
}

- (simd_float4x4)globalTransform {
    simd_float4x4 localTransform = self.localTransform;
    simd_float4x4 ancestorTransform = self.parent ? self.parent.globalTransform : matrix_identity_float4x4;
    return matrix_multiply(ancestorTransform, localTransform);
}

- (void)setLocalTransform:(simd_float4x4)localTransform {
    _localTransform = localTransform;
}

- (simd_float4x4)localTransform {
    if (self.localTransformIsDirty) {
        [self computeLocalTransform];
    }
    
    return _localTransform;
}

- (void)computeLocalTransform {
    simd_float4x4 translationMatrix = matrix_identity_float4x4;
    translationMatrix.columns[3][0] = _translation[0];
    translationMatrix.columns[3][1] = _translation[1];
    translationMatrix.columns[3][2] = _translation[2];

    simd_float4x4 rotationMatrix = simd_matrix4x4(_rotationQuaternion);
    
    simd_float4x4 scaleMatrix = matrix_identity_float4x4;
    scaleMatrix.columns[0][0] = _scale[0];
    scaleMatrix.columns[1][1] = _scale[1];
    scaleMatrix.columns[2][2] = _scale[2];
    
    _localTransform = matrix_multiply(matrix_multiply(translationMatrix, rotationMatrix), scaleMatrix);
    _localTransformDirty = NO;
}

- (GLTFBoundingBox)approximateBounds {
    return [self _approximateBoundsRecursive:matrix_identity_float4x4];
}

- (GLTFBoundingBox)_approximateBoundsRecursive:(simd_float4x4)transform {
    GLTFBoundingBox bounds = { 0 };
    
    if (self.mesh != nil) {
        for (GLTFSubmesh *submesh in self.mesh.submeshes) {
            GLTFBoundingBox submeshBounds = { 0 };
            GLTFAccessor *positionAccessor = submesh.accessorsForAttributes[GLTFAttributeSemanticPosition];
            GLTFValueRange positionRange = positionAccessor.valueRange;
            submeshBounds.minPoint.x = positionRange.minValue[0];
            submeshBounds.minPoint.y = positionRange.minValue[1];
            submeshBounds.minPoint.z = positionRange.minValue[2];
            submeshBounds.maxPoint.x = positionRange.maxValue[0];
            submeshBounds.maxPoint.y = positionRange.maxValue[1];
            submeshBounds.maxPoint.z = positionRange.maxValue[2];
            GLTFBoundingBoxUnion(&bounds, submeshBounds);
        }
    }
    
    simd_float4x4 globalTransform = matrix_multiply(transform, self.localTransform);
    
    GLTFBoundingBoxTransform(&bounds, globalTransform);
    
    for (GLTFNode *child in self.children) {
        GLTFBoundingBox childBounds = [child _approximateBoundsRecursive:globalTransform];
        GLTFBoundingBoxUnion(&bounds, childBounds);
    }
    
    return bounds;
}

- (void)acceptVisitor:(GLTFNodeVisitor)visitor strategy:(GLTFVisitationStrategy)strategy {
    [self _acceptVisitor:visitor depth:0 strategy:strategy];
}

- (void)_acceptVisitor:(GLTFNodeVisitor)visitor depth:(int)depth strategy:(GLTFVisitationStrategy)strategy {
    switch (strategy) {
        case GLTFVisitationStrategyDepthFirst:
        default:
        {
            BOOL recurse = YES;
            visitor(self, depth, &recurse);
            if (recurse) {
                for (GLTFNode *child in self.children) {
                    [child _acceptVisitor:visitor depth:(depth + 1) strategy:strategy];
                }
            }
        }
    }
}

@end
