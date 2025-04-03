//
//  GLTFModelLoader.h
//  GLTFTestRendering
//
//  Created by Malcolm Toon on 11/25/24.
//

#import <Foundation/Foundation.h>
#import "gltfkit/GLTFAsset.h"

NS_ASSUME_NONNULL_BEGIN

typedef void (^GLTFModelLoaderCompletionHandler)(GLTFAsset *asset);

@interface GLTFModelLoader : NSObject

- (void)loadURL:(NSURL *)assetURL
    withCompletionHandler:(GLTFModelLoaderCompletionHandler)completionHandler
          bufferAllocator:(id<GLTFBufferAllocator>)bufferAllocator;

@end

NS_ASSUME_NONNULL_END
