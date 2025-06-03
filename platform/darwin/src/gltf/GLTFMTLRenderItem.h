//
//  GLTFMTLRenderItem.h
//  GLTFTestRendering
//
//  Created by Malcolm Toon on 11/19/24.
//

#import <Foundation/Foundation.h>
#import "gltfkit/GLTFMesh.h"
#import "gltfkit/GLTFNode.h"

NS_ASSUME_NONNULL_BEGIN

@interface GLTFMTLRenderItem : NSObject
@property (nonatomic) NSString *label;
@property (nonatomic) GLTFNode *node;
@property (nonatomic, strong) GLTFSubmesh *submesh;
@property (nonatomic, assign) VertexUniforms vertexUniforms;
@property (nonatomic, assign) FragmentUniforms fragmentUniforms;
@end

NS_ASSUME_NONNULL_END
