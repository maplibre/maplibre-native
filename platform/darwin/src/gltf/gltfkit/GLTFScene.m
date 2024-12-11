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

#import "GLTFScene.h"
#import "GLTFNode.h"

@interface GLTFScene ()
@property (nonatomic, strong) NSMutableArray *mutableNodes;
@end

@implementation GLTFScene

- (void)setNodes:(NSArray<GLTFNode *> *)nodes {
    _mutableNodes = [nodes mutableCopy];
}

- (NSArray<GLTFNode *> *)nodes {
    return [_mutableNodes copy];
}

- (void)addNode:(GLTFNode *)node {
    if (node.parent) {
        [node removeFromParent];
    }
    
    [_mutableNodes addObject:node];
}

- (GLTFBoundingBox)approximateBounds {
    GLTFBoundingBox sceneBounds = { 0 };
    for (GLTFNode *node in self.nodes) {
        GLTFBoundingBox nodeBounds = node.approximateBounds;
        GLTFBoundingBoxUnion(&sceneBounds, nodeBounds);
    }
    return sceneBounds;
}

- (void)acceptVisitor:(GLTFNodeVisitor)visitor strategy:(GLTFVisitationStrategy)strategy {
    for (GLTFNode *node in self.nodes) {
        [node acceptVisitor:visitor strategy:strategy];
    }
}

@end
