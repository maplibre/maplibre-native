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

#import "GLTFBufferView.h"
#import "GLTFObject.h"

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface GLTFImage : GLTFObject

/// A reference to a buffer view containing image data, if url is nil
@property (nonatomic, strong) GLTFBufferView *_Nullable bufferView;

/// The MIME type of the data contained in this image's buffer view
@property (nonatomic, copy) NSString *_Nullable mimeType;

/// A file URL, if the URI was not a decodable data-uri; otherwise nil
@property (nonatomic, copy) NSURL *_Nullable url;

/// A data object containing the data encoded in the image's data-uri, if present; otherwise nil
@property (nonatomic, strong) NSData *imageData;

@end

NS_ASSUME_NONNULL_END
