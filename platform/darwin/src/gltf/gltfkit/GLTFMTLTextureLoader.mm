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

#import "GLTFMTLTextureLoader.h"
#import <Accelerate/Accelerate.h>
#import <CoreGraphics/CoreGraphics.h>
#import <ImageIO/ImageIO.h>

NSString *const GLTFMTLTextureLoaderOptionGenerateMipmaps = @"GLTFMTLTextureLoaderOptionGenerateMipmaps";
NSString *const GLTFMTLTextureLoaderOptionUsageFlags = @"GLTFMTLTextureLoaderOptionUsageFlags";
NSString *const GLTFMTLTextureLoaderOptionSRGB = @"GLTFMTLTextureLoaderOptionSRGB";

__fp16 *GLTFMTLConvertImageToRGBA16F(CGImageRef image)
{
    size_t width = CGImageGetWidth(image);
    size_t height = CGImageGetHeight(image);

    void *dstPixels = malloc(sizeof(__fp16) * 4 * width * height);
    size_t dstBytesPerRow = sizeof(__fp16) * 4 * width;
    vImage_Buffer dstBuffer = {
        .data = dstPixels,
        .height = height,
        .width = width,
        .rowBytes = dstBytesPerRow
    };

    vImage_CGImageFormat srcFormat = {
        .bitsPerComponent = (uint32_t)CGImageGetBitsPerComponent(image),
        .bitsPerPixel = (uint32_t)CGImageGetBitsPerPixel(image),
        .colorSpace = CGImageGetColorSpace(image),
        .bitmapInfo = CGImageGetBitmapInfo(image)
    };

    vImage_CGImageFormat dstFormat = {
        .bitsPerComponent = sizeof(__fp16) * 8,
        .bitsPerPixel = sizeof(__fp16) * 8 * 4,
        .colorSpace = CGImageGetColorSpace(image),
        .bitmapInfo = kCGBitmapByteOrder16Little | kCGBitmapFloatComponents | kCGImageAlphaLast
    };

    vImage_Error error = kvImageNoError;
    CGFloat background[] = { 0, 0, 0, 1 };
    vImageConverterRef converter = vImageConverter_CreateWithCGImageFormat(&srcFormat,
                                                                           &dstFormat,
                                                                           background,
                                                                           kvImageNoFlags,
                                                                           &error);

    CGDataProviderRef dataProvider = CGImageGetDataProvider(image);
    CFDataRef srcData = CGDataProviderCopyData(dataProvider);

    const void *srcPixels = CFDataGetBytePtr(srcData);
    size_t srcBytesPerRow = CGImageGetBytesPerRow(image);

    vImage_Buffer srcBuffer = {
        .data = (void *)srcPixels,
        .height = height,
        .width = width,
        .rowBytes = srcBytesPerRow
    };

    error = vImageConvert_AnyToAny(converter, &srcBuffer, &dstBuffer, NULL, kvImageNoFlags);

    vImageConverter_Release(converter);
    CFRelease(srcData);

    return (__fp16*)dstPixels;
}

unsigned char *GLTFMTLConvertImageToRGBA8U(CGImageRef image)
{
    size_t width = CGImageGetWidth(image);
    size_t height = CGImageGetHeight(image);
    
    CGColorSpaceRef srcColorSpace = CGImageGetColorSpace(image);

    vImage_CGImageFormat srcFormat = {
        .bitsPerComponent = (uint32_t)CGImageGetBitsPerComponent(image),
        .bitsPerPixel = (uint32_t)CGImageGetBitsPerPixel(image),
        .colorSpace = srcColorSpace,
        .bitmapInfo = CGImageGetBitmapInfo(image)
    };

    void *dstPixels = malloc(sizeof(unsigned char) * 4 * width * height);
    vImage_Buffer dstBuffer = {
        .data = dstPixels,
        .height = height,
        .width = width,
        .rowBytes = sizeof(unsigned char) * 4 * width
    };
    
    CGColorSpaceRef dstColorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);

    vImage_CGImageFormat dstFormat = {
        .bitsPerComponent = sizeof(unsigned char) * 8,
        .bitsPerPixel = sizeof(unsigned char) * 8 * 4,
        .colorSpace = dstColorSpace,
        .bitmapInfo = (CGBitmapInfo)kCGBitmapByteOrder32Big | (CGBitmapInfo)kCGImageAlphaLast
    };
    
    vImage_Error error = kvImageNoError;
    CGFloat background[] = { 0, 0, 0, 1 };
    vImageConverterRef converter = vImageConverter_CreateWithCGImageFormat(&srcFormat,
                                                                           &dstFormat,
                                                                           background,
                                                                           kvImageNoFlags,
                                                                           &error);
    
    CGDataProviderRef dataProvider = CGImageGetDataProvider(image);
    CFDataRef srcData = CGDataProviderCopyData(dataProvider);
    
    const void *srcPixels = CFDataGetBytePtr(srcData);
    
    size_t srcBytesPerPixel = CGImageGetBitsPerPixel(image) / 8;
    
    vImage_Buffer srcBuffer = {
        .data = (void *)srcPixels,
        .height = height,
        .width = width,
        .rowBytes = srcBytesPerPixel * width
    };

    vImageConvert_AnyToAny(converter, &srcBuffer, &dstBuffer, NULL, kvImageNoFlags);
    
    vImageConverter_Release(converter);
    CFRelease(srcData);
    
    return (unsigned char*)dstPixels;
}

@interface GLTFMTLTextureLoader ()
@property (nonatomic, strong) id<MTLDevice> device;
@property (nonatomic, strong) id<MTLCommandQueue> commandQueue;
@end

@implementation GLTFMTLTextureLoader

- (instancetype)initWithDevice:(id<MTLDevice>)device {
    if ((self = [super init])) {
        _device = device;
        _commandQueue = [device newCommandQueue];
    }
    return self;
}

- (id<MTLTexture>)newTextureWithContentsOfURL:(NSURL *)url options:(NSDictionary *)options error:(NSError **)error {
    if (url == nil) {
        return nil;
    }

    NSData *data = [NSData dataWithContentsOfURL:url];

    return [self newTextureWithData:data options:options error:error];
}

- (id<MTLTexture>)newTextureWithData:(NSData *)data options:(NSDictionary *)options error:(NSError **)error {
    if (data == nil) {
        return nil;
    }
    
    NSNumber *sRGBOption = options[GLTFMTLTextureLoaderOptionSRGB];
    BOOL sRGB = (sRGBOption != nil) ? sRGBOption.boolValue : NO;

    CGImageSourceRef imageSource = CGImageSourceCreateWithData((__bridge CFDataRef)data, nil);
    CGImageRef image = CGImageSourceCreateImageAtIndex(imageSource, 0, nil);
    size_t width = CGImageGetWidth(image);
    size_t height = CGImageGetHeight(image);
    size_t bitsPerComponent = CGImageGetBitsPerComponent(image);

    void *dstBytes = NULL;
    MTLPixelFormat pixelFormat = MTLPixelFormatInvalid;
    if (bitsPerComponent == 8) {
        pixelFormat = sRGB ? MTLPixelFormatRGBA8Unorm_sRGB : MTLPixelFormatRGBA8Unorm;
        dstBytes = GLTFMTLConvertImageToRGBA8U(image);
        bitsPerComponent = 8;
    } else if (bitsPerComponent == 16 || bitsPerComponent == 32) {
        pixelFormat = MTLPixelFormatRGBA16Float;
        dstBytes = GLTFMTLConvertImageToRGBA16F(image);
        bitsPerComponent = 16;
    }

    size_t bytesPerRow = (bitsPerComponent / 8) * 4 * width;
    NSNumber *mipmapOption = options[GLTFMTLTextureLoaderOptionGenerateMipmaps];
    BOOL mipmapped = (mipmapOption != nil) ? mipmapOption.boolValue : NO;

    MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixelFormat
                                                                                          width:width
                                                                                         height:height
                                                                                      mipmapped:mipmapped];

    id<MTLTexture> texture = [self newTextureWithBytes:(const unsigned char*)dstBytes
                                           bytesPerRow:bytesPerRow
                                            descriptor:descriptor
                                               options:options
                                                 error:error];
    
    free(dstBytes);
    CGImageRelease(image);
    CFRelease(imageSource);

    return texture;
}

- (id<MTLTexture> _Nullable)newTextureWithBytes:(const unsigned char *)bytes
                                    bytesPerRow:(size_t)bytesPerRow
                                     descriptor:(MTLTextureDescriptor *)descriptor
                                        options:(NSDictionary * _Nullable)options
                                          error:(NSError **)error
{
    NSNumber *usageOption = options[GLTFMTLTextureLoaderOptionUsageFlags];
    descriptor.usage = (usageOption != nil) ? usageOption.integerValue : MTLTextureUsageShaderRead;
    
    id<MTLTexture> texture = [self.device newTextureWithDescriptor:descriptor];
    
    [texture replaceRegion:MTLRegionMake2D(0, 0, texture.width, texture.height)
               mipmapLevel:0
                 withBytes:bytes
               bytesPerRow:bytesPerRow];

    if (texture != nil && (texture.mipmapLevelCount > 1)) {
        id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];
        id<MTLBlitCommandEncoder> commandEncoder = [commandBuffer blitCommandEncoder];
        [commandEncoder generateMipmapsForTexture:texture];
        [commandEncoder endEncoding];
        [commandBuffer commit];
    }
    
    return texture;
}

@end
