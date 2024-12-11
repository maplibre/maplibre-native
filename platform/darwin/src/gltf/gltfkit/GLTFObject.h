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

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol GLTFObject

/// The user-defined name of this object. Need not be unique.
@property (nonatomic, copy) NSString * _Nullable name;
/// Data specific to any extensions used in this document
@property (nonatomic, copy) NSDictionary *extensions;
/// Contains application-specific information that is passed through but not parsed
@property (nonatomic, copy) NSDictionary *extras;

@end

@interface GLTFObject : NSObject <GLTFObject>

/// A unique identifier for this object
@property (nonatomic, readonly) NSUUID *identifier;
/// The user-defined name of this object. Need not be unique.
@property (nonatomic, copy) NSString * _Nullable name;
/// Data specific to any extensions used in this document
@property (nonatomic, copy) NSDictionary *extensions;
/// Contains application-specific information that is passed through but not parsed
@property (nonatomic, copy) NSDictionary *extras;

@end

NS_ASSUME_NONNULL_END
