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

#import "GLTFMTLBufferAllocator.h"

#import <Metal/Metal.h>

static uint64_t _liveAllocationSize;

@interface GLTFMTLBufferAllocator ()
+ (void)incrementLiveAllocationSizeByLength:(uint64_t)length;
+ (void)decrementLiveAllocationSizeByLength:(uint64_t)length;
@end

@interface GLTFMTLBuffer ()
@property (nonatomic, strong) id<MTLBuffer> buffer;

- (instancetype)initWithBuffer:(id<MTLBuffer>)buffer;

@end

@implementation GLTFMTLBuffer

@synthesize name;
@synthesize extras;
@synthesize extensions;

- (instancetype)initWithBuffer:(id<MTLBuffer>)buffer {
    if ((self = [super init])) {
        _buffer = buffer;
        [GLTFMTLBufferAllocator incrementLiveAllocationSizeByLength:_buffer.length];
    }
    return self;
}

- (void)dealloc {
    [GLTFMTLBufferAllocator decrementLiveAllocationSizeByLength:_buffer.length];
}

- (NSInteger)length {
    return [self.buffer length];
}

- (void *)contents {
    return [self.buffer contents];
}

@end

@interface GLTFMTLBufferAllocator ()
@property (nonatomic, strong) id<MTLDevice> device;
@end

@implementation GLTFMTLBufferAllocator

+ (void)incrementLiveAllocationSizeByLength:(uint64_t)length {
    _liveAllocationSize += length;
}

+ (void)decrementLiveAllocationSizeByLength:(uint64_t)length {
    _liveAllocationSize -= length;
}

+ (uint64_t)liveAllocationSize {
    return _liveAllocationSize;
}

- (instancetype)initWithDevice:(id<MTLDevice>)device {
    if ((self = [super init])) {
        _device = device;
    }
    return self;
}

- (id<GLTFBuffer>)newBufferWithLength:(NSInteger)length {
    MTLResourceOptions options = MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeShared;
    id<MTLBuffer> underlying = [self.device newBufferWithLength:length options:options];
    return [[GLTFMTLBuffer alloc] initWithBuffer:underlying];
}


- (id<GLTFBuffer>)newBufferWithData:(NSData *)data {
    MTLResourceOptions options = MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeShared;
    id<MTLBuffer> underlying = [self.device newBufferWithBytes:data.bytes length:data.length options:options];
    return [[GLTFMTLBuffer alloc] initWithBuffer:underlying];
}

@end
