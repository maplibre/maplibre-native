#import "MLNTerrain.h"
#import "MLNTerrain_Private.h"

#import <mln/style/terrain.hpp>

@implementation MLNTerrain

- (instancetype)initWithSourceIdentifier:(NSString *)sourceIdentifier
                             exaggeration:(CGFloat)exaggeration {
  if (self = [super init]) {
    _sourceIdentifier = [sourceIdentifier copy];
    _exaggeration = exaggeration;
  }
  return self;
}

- (instancetype)initWithSourceIdentifier:(NSString *)sourceIdentifier {
  return [self initWithSourceIdentifier:sourceIdentifier exaggeration:1.0];
}

- (instancetype)initWithMBGLTerrain:(const mln::style::Terrain &)mbglTerrain {
  return [self initWithSourceIdentifier:@(mbglTerrain.getSource().c_str())
                            exaggeration:mbglTerrain.getExaggeration()];
}

- (mln::style::Terrain)mbglTerrain {
  return mln::style::Terrain(self.sourceIdentifier.UTF8String, static_cast<float>(self.exaggeration));
}

- (id)copyWithZone:(nullable NSZone *)zone {
  return [[MLNTerrain allocWithZone:zone] initWithSourceIdentifier:self.sourceIdentifier
                                                       exaggeration:self.exaggeration];
}

- (BOOL)isEqual:(id)other {
  if (self == other) {
    return YES;
  }
  if (![other isKindOfClass:[MLNTerrain class]]) {
    return NO;
  }

  MLNTerrain *otherTerrain = other;
  return [self.sourceIdentifier isEqualToString:otherTerrain.sourceIdentifier] &&
         self.exaggeration == otherTerrain.exaggeration;
}

- (NSUInteger)hash {
  return self.sourceIdentifier.hash ^ @(self.exaggeration).hash;
}

- (NSString *)description {
  return [NSString stringWithFormat:@"<%@: %p; sourceIdentifier = %@; exaggeration = %.2f>",
                                    NSStringFromClass([self class]), (void *)self,
                                    self.sourceIdentifier, self.exaggeration];
}

@end
