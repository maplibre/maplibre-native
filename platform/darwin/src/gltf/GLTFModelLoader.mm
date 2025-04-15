//
//  GLTFModelLoader.m
//  GLTFTestRendering
//
//  Created by Malcolm Toon on 11/25/24.
//

#import "GLTFModelLoader.h"
#import "gltfkit/GLTFAsset.h"
#import "gltfkit/GLTFMTLBufferAllocator.h"

@interface GLTFModelLoader () <GLTFAssetLoadingDelegate> {
    
}

@property GLTFModelLoaderCompletionHandler completionHandler;

@end

@implementation GLTFModelLoader

-(void)loadURL:(NSURL *)assetURL
withCompletionHandler:(GLTFModelLoaderCompletionHandler)completionHandler
bufferAllocator:(id<GLTFBufferAllocator>)bufferAllocator {
    
    self.completionHandler = completionHandler;
    [GLTFAsset loadAssetWithURL:assetURL
                bufferAllocator:bufferAllocator
                       delegate:self];
    
}


- (void)assetWithURL:(nonnull NSURL *)assetURL didFailToLoadWithError:(nonnull NSError *)error {
    NSLog(@"Asset load failed with error: %@", error);
}

- (void)assetWithURL:(nonnull NSURL *)assetURL
    didFinishLoading:(nonnull GLTFAsset *)asset {
    dispatch_async(dispatch_get_main_queue(), ^{
        
        if (self.completionHandler) {
            self.completionHandler(asset);
        }
        self.completionHandler = nil;
        
        NSLog(@"INFO: Total live buffer allocation size after document load is %0.2f MB", ([GLTFMTLBufferAllocator liveAllocationSize] / (float)1e6));
    });
}

- (void)assetWithURL:(nonnull NSURL *)assetURL
requiresContentsOfURL:(nonnull NSURL *)url
   completionHandler:(void (^)(NSData *_Nullable, NSError *_Nullable))completionHandler {
    NSURLSessionDataTask *task = [GLTFModelLoader.urlSession dataTaskWithURL:url
                                                        completionHandler:^(NSData *data, NSURLResponse *response, NSError *error)
    {
        completionHandler(data, error);
    }];
    [task resume];
}

+ (NSURLSession *)urlSession {
    static NSURLSession *_urlSession = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        NSURLSessionConfiguration *configuration = [NSURLSessionConfiguration defaultSessionConfiguration];
        _urlSession = [NSURLSession sessionWithConfiguration:configuration];
    });
    return _urlSession;
}



@end
