#import "MLNTerrain.h"
#import "MLNTerrain_Private.h"

@implementation MLNTerrain

- (instancetype)initWithSourceIdentifier:(NSString *)sourceIdentifier
                            exaggeration:(CGFloat)exaggeration {
  if (self = [super init]) {
    _sourceIdentifier = [sourceIdentifier copy];
    // The renderer stores a float. Rounding here keeps a terrain equal to the one read back.
    _exaggeration = static_cast<float>(exaggeration);
  }
  return self;
}

- (instancetype)initWithSourceIdentifier:(NSString *)sourceIdentifier {
  return [self initWithSourceIdentifier:sourceIdentifier exaggeration:1];
}

- (instancetype)initWithMBGLTerrain:(const mln::style::Terrain &)mbglTerrain {
  return [self initWithSourceIdentifier:@(mbglTerrain.getSource().c_str())
                           exaggeration:mbglTerrain.getExaggeration()];
}

- (mln::style::Terrain)mbglTerrain {
  return mln::style::Terrain(self.sourceIdentifier.UTF8String ?: "",
                             static_cast<float>(self.exaggeration));
}

- (id)copyWithZone:(nullable NSZone *)zone {
  return self;
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
  return self.sourceIdentifier.hash * 31 + @(self.exaggeration).hash;
}

- (NSString *)description {
  return [NSString stringWithFormat:@"<%@: %p; sourceIdentifier = \"%@\"; exaggeration = %g>",
                                    NSStringFromClass([self class]), (void *)self,
                                    self.sourceIdentifier, self.exaggeration];
}

@end
