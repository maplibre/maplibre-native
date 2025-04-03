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

#import "GLTFImage.h"

#import <CoreGraphics/CoreGraphics.h>

@implementation GLTFImage

+ (CGImageRef)newImageForData:(NSData *)data mimeType:(NSString *)mimeType {
    CGImageRef image = NULL;
    CGDataProviderRef provider = CGDataProviderCreateWithCFData((__bridge CFDataRef)data);
    if ([mimeType isEqualToString:@"image/jpeg"]) {
        image = CGImageCreateWithJPEGDataProvider(provider, NULL, false, kCGRenderingIntentDefault);
    } else if ([mimeType isEqualToString:@"image/png"]) {
        image = CGImageCreateWithPNGDataProvider(provider, NULL, false, kCGRenderingIntentDefault);
    } else {
        NSLog(@"Unknown MIME type encountered when decoding image: %@", mimeType);
    }
    CGDataProviderRelease(provider);
    return image;
}

+ (CGImageRef)newImageForDataURI:(NSString *)uriData {
    NSString *prefix = @"data:";
    if ([uriData hasPrefix:prefix]) {
        NSInteger prefixEnd = prefix.length;
        NSInteger firstComma = [uriData rangeOfString:@","].location;
        if (firstComma != NSNotFound) {
            NSString *mediaTypeAndTokenString = [uriData substringWithRange:NSMakeRange(prefixEnd, firstComma - prefixEnd)];
            NSArray *mediaTypeAndToken = [mediaTypeAndTokenString componentsSeparatedByString:@";"];
            if (mediaTypeAndToken.count > 0) {
                NSString *mediaType = mediaTypeAndToken.firstObject;
                NSString *encodedImageData = [uriData substringFromIndex:firstComma + 1];
                NSData *imageData = [[NSData alloc] initWithBase64EncodedString:encodedImageData
                                                                        options:NSDataBase64DecodingIgnoreUnknownCharacters];
                return [self newImageForData:imageData mimeType:mediaType];
            }
        }
    }
    return NULL;
}

@end
