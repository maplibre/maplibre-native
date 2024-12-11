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

#import "GLTFTextureSampler.h"

@implementation GLTFTextureSampler

- (instancetype)init {
    if ((self = [super init])) {
        _magFilter = GLTFSamplingFilterLinear;
        _minFilter = GLTFSamplingFilterNearestMipLinear;
        _sAddressMode = GLTFAddressModeRepeat;
        _tAddressMode = GLTFAddressModeRepeat;
    }
    return self;
}

- (BOOL)isEqual:(id)other {
    if (![other isKindOfClass:[GLTFTextureSampler class]]) {
        return NO;
    }
    
    GLTFTextureSampler *otherSampler = (GLTFTextureSampler *)other;
    BOOL areEqual =
        (otherSampler.magFilter == self.magFilter) &&
        (otherSampler.minFilter == self.minFilter) &&
        (otherSampler.sAddressMode == self.sAddressMode) &&
        (otherSampler.tAddressMode == self.tAddressMode);
    return areEqual;
}

- (NSUInteger)hash {
    NSUInteger hashValue = ((_sAddressMode << 16) | _tAddressMode) + ((_magFilter << 16) | _minFilter);
    return hashValue;
}

- (id)copyWithZone:(NSZone *)zone {
    GLTFTextureSampler *copy = [[GLTFTextureSampler allocWithZone:zone] init];
    copy.magFilter = self.magFilter;
    copy.minFilter = self.minFilter;
    copy.sAddressMode = self.sAddressMode;
    copy.tAddressMode = self.tAddressMode;
    return copy;
}

@end

