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

#import <Metal/Metal.h>

NS_ASSUME_NONNULL_BEGIN

extern NSString *const GLTFMTLTextureLoaderOptionGenerateMipmaps;
extern NSString *const GLTFMTLTextureLoaderOptionUsageFlags;
extern NSString *const GLTFMTLTextureLoaderOptionSRGB;

@interface GLTFMTLTextureLoader : NSObject
- (instancetype)initWithDevice:(id<MTLDevice>)device;
- (id<MTLTexture> _Nullable)newTextureWithContentsOfURL:(NSURL *)url
                                                options:(NSDictionary *_Nullable)options
                                                  error:(NSError **)error;
- (id<MTLTexture> _Nullable)newTextureWithData:(NSData *)data
                                       options:(NSDictionary *_Nullable)options
                                         error:(NSError **)error;
@end

NS_ASSUME_NONNULL_END
