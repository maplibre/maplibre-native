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
#import "GLTFEnums.h"

NS_ASSUME_NONNULL_BEGIN

extern const NSInteger GLTFVertexDescriptorMaxAttributeCount;
extern const NSInteger GLTFVertexDescriptorMaxBufferLayoutCount;

extern NSString *const GLTFAttributeSemanticPosition;
extern NSString *const GLTFAttributeSemanticTangent;
extern NSString *const GLTFAttributeSemanticNormal;
extern NSString *const GLTFAttributeSemanticTexCoord0;
extern NSString *const GLTFAttributeSemanticTexCoord1;
extern NSString *const GLTFAttributeSemanticColor0;
extern NSString *const GLTFAttributeSemanticJoints0;
extern NSString *const GLTFAttributeSemanticJoints1;
extern NSString *const GLTFAttributeSemanticWeights0;
extern NSString *const GLTFAttributeSemanticWeights1;
extern NSString *const GLTFAttributeSemanticRoughness;
extern NSString *const GLTFAttributeSemanticMetallic;

@interface GLTFVertexAttribute : NSObject
@property (nonatomic, copy) NSString *semantic;
@property (nonatomic, assign) GLTFDataType componentType;
@property (nonatomic, assign) GLTFDataDimension dimension;
@property (nonatomic, assign) NSInteger offset;
@end

@interface GLTFBufferLayout : NSObject
@property (nonatomic, assign) NSInteger stride;
@end

@interface GLTFVertexDescriptor : NSObject
@property (nonatomic, copy) NSArray<GLTFVertexAttribute *> *attributes;
@property (nonatomic, copy) NSArray<GLTFBufferLayout *> *bufferLayouts;
@end

NS_ASSUME_NONNULL_END
