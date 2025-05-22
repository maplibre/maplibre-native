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

#import "GLTFVertexDescriptor.h"

const NSInteger GLTFVertexDescriptorMaxAttributeCount = 16;
const NSInteger GLTFVertexDescriptorMaxBufferLayoutCount = 16;

NSString *const GLTFAttributeSemanticPosition  = @"POSITION";
NSString *const GLTFAttributeSemanticTangent   = @"TANGENT";
NSString *const GLTFAttributeSemanticNormal    = @"NORMAL";
NSString *const GLTFAttributeSemanticTexCoord0 = @"TEXCOORD_0";
NSString *const GLTFAttributeSemanticTexCoord1 = @"TEXCOORD_1";
NSString *const GLTFAttributeSemanticColor0    = @"COLOR_0";
NSString *const GLTFAttributeSemanticJoints0   = @"JOINTS_0";
NSString *const GLTFAttributeSemanticJoints1   = @"JOINTS_1";
NSString *const GLTFAttributeSemanticWeights0  = @"WEIGHTS_0";
NSString *const GLTFAttributeSemanticWeights1  = @"WEIGHTS_1";
NSString *const GLTFAttributeSemanticRoughness = @"ROUGHNESS";
NSString *const GLTFAttributeSemanticMetallic  = @"METALLIC";

@implementation GLTFVertexAttribute

- (NSString *)description {
    return [NSString stringWithFormat:@"GLTFVertexAttribute: component type: %d, count: %d, offset %d [[%@]]",
            (int)self.componentType, (int)self.dimension, (int)self.offset, self.semantic];
}

@end

@implementation GLTFBufferLayout
@end

@implementation GLTFVertexDescriptor

- (instancetype)init {
    if ((self = [super init])) {
        NSMutableArray *mutableAttributes = [NSMutableArray arrayWithCapacity:GLTFVertexDescriptorMaxAttributeCount];
        for (int i = 0; i < GLTFVertexDescriptorMaxAttributeCount; ++i) {
            [mutableAttributes addObject: [GLTFVertexAttribute new]];
        }
        _attributes = [mutableAttributes copy];
        
        NSMutableArray *mutableLayouts = [NSMutableArray arrayWithCapacity:GLTFVertexDescriptorMaxBufferLayoutCount];
        for (int i = 0; i < GLTFVertexDescriptorMaxBufferLayoutCount; ++i) {
            [mutableLayouts addObject: [GLTFBufferLayout new]];
        }
        _bufferLayouts = [mutableLayouts copy];
    }
    return self;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"GLTFVertexDescriptor:\nattributes: %@\nlayouts: %@",
            self.attributes, self.bufferLayouts];
}

@end
