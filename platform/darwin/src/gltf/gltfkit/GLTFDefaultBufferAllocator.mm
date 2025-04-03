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

#import "GLTFDefaultBufferAllocator.h"

static uint64_t _liveAllocationSize;

@interface GLTFDefaultBufferAllocator ()
+ (void)incrementLiveAllocationSizeByLength:(uint64_t)length;
+ (void)decrementLiveAllocationSizeByLength:(uint64_t)length;
@end

@interface GLTFMemoryBuffer: NSObject <GLTFBuffer>
@property (nonatomic, assign) void *bytes;
@property (nonatomic, assign) NSInteger length;
- (instancetype)initWithLength:(NSInteger)length;
- (instancetype)initWithData:(NSData *)data;
@end

@implementation GLTFMemoryBuffer

@synthesize name;
@synthesize extras;
@synthesize extensions;

- (instancetype)initWithLength:(NSInteger)length {
    if ((self = [super init])) {
        _bytes = malloc(length);
        _length = length;
        [GLTFDefaultBufferAllocator incrementLiveAllocationSizeByLength:_length];
    }
    return self;
}

- (instancetype)initWithData:(NSData *)data {
    if ((self = [super init])) {
        _length = data.length;
        _bytes = malloc(_length);
        memcpy(_bytes, data.bytes, _length);
        [GLTFDefaultBufferAllocator incrementLiveAllocationSizeByLength:_length];
    }
    return self;
}

- (void)dealloc {
    free(_bytes);
    [GLTFDefaultBufferAllocator decrementLiveAllocationSizeByLength:_length];
}

- (void *)contents NS_RETURNS_INNER_POINTER {
    return self.bytes;
}

@end

@implementation GLTFDefaultBufferAllocator

+ (void)incrementLiveAllocationSizeByLength:(uint64_t)length {
    _liveAllocationSize += length;
}

+ (void)decrementLiveAllocationSizeByLength:(uint64_t)length {
    _liveAllocationSize -= length;
}

+ (uint64_t)liveAllocationSize {
    return _liveAllocationSize;
}

- (id<GLTFBuffer>)newBufferWithLength:(NSInteger)length {
    GLTFMemoryBuffer *buffer = [[GLTFMemoryBuffer alloc] initWithLength:length];
    return buffer;
}

- (id<GLTFBuffer>)newBufferWithData:(NSData *)data {
    GLTFMemoryBuffer *buffer = [[GLTFMemoryBuffer alloc] initWithData:data];
    return buffer;
}

@end
