#import "MLNPluginLayer.h"
#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/mtl/texture2d.hpp>
#include <memory>
#import "MLNPluginLayer_Private.h"

@implementation MLNPluginLayerTileFeature
@end

@implementation MLNPluginLayerTileFeatureCollection
@end

@implementation MLNPluginLayerFeatureCoordinate
@end

//@implementation MLNRect
//@end

@implementation MLNPluginAtlas
@end

@interface MLNTexture () {
  std::shared_ptr<mbgl::gfx::Texture2D> _texture;
}
@end

@implementation MLNTexture

- (instancetype)initWithTexture:(std::shared_ptr<mbgl::gfx::Texture2D>)texture {
  self = [super init];
  if (self) {
    _texture = texture;
  }
  return self;
}

+ (instancetype)textureWithTexture:(std::shared_ptr<mbgl::gfx::Texture2D>)texture {
  return [[self alloc] initWithTexture:texture];
}

- (void)setTexture:(std::shared_ptr<mbgl::gfx::Texture2D>)texture {
  _texture = texture;
}

- (std::shared_ptr<mbgl::gfx::Texture2D>)getTexture {
  return _texture;
}

- (CGSize)getSize {
  mbgl::Size size = _texture->getSize();
  CGSize cgSize = CGSizeMake(size.width, size.height);
  return cgSize;
}

- (id<MTLTexture>)getMetalTexture {
  const auto &tex = static_pointer_cast<mbgl::mtl::Texture2D>(_texture);
  return (__bridge id<MTLTexture>)tex->getMetalTexture();
}

@end

@implementation MLNQuad

- (instancetype)initWithSymbolQuad:(const mbgl::SymbolQuad &)quad {
  self = [super init];
  if (self) {
    _tl = CGPointMake(quad.tl.x, quad.tl.y);
    _tr = CGPointMake(quad.tr.x, quad.tr.y);
    _bl = CGPointMake(quad.bl.x, quad.bl.y);
    _br = CGPointMake(quad.br.x, quad.br.y);
    _texCoords = CGRectMake(quad.tex.x, quad.tex.y, quad.tex.w, quad.tex.h);
    _offset = CGPointMake(quad.glyphOffset.x, quad.glyphOffset.y);
  }
  return self;
}

+ (instancetype)quadWithSymbolQuad:(const mbgl::SymbolQuad &)quad {
  return [[self alloc] initWithSymbolQuad:quad];
}

- (instancetype)initWithQuad:(MLNQuad *)quad {
  self = [super init];
  if (self) {
    _tl = CGPointMake(quad.tl.x, quad.tl.y);
    _tr = CGPointMake(quad.tr.x, quad.tr.y);
    _bl = CGPointMake(quad.bl.x, quad.bl.y);
    _br = CGPointMake(quad.br.x, quad.br.y);
    _texCoords = quad.texCoords;
    _offset = quad.offset;
  }

  return self;
}

+ (instancetype)quadWithQuad:(MLNQuad *)quad {
  return [[self alloc] initWithQuad:quad];
}

@end

@implementation MLNPluginLayerFeatureSymbolProperty

- (instancetype)initWithName:(NSString *)name type:(MLNPluginLayerFeatureSymbolPropertyType)type {
  self = [super init];
  if (self) {
    _name = name;
    _type = type;
  }
  return self;
}

+ (instancetype)propertyWithName:(NSString *)name
                            type:(MLNPluginLayerFeatureSymbolPropertyType)type {
  return [[self alloc] initWithName:name type:type];
}

@end

@implementation MLNPluginLayerProperty

+ (MLNPluginLayerProperty *)propertyWithName:(NSString *)propertyName
                                propertyType:(MLNPluginLayerPropertyType)propertyType
                                defaultValue:(id)defaultValue {
  MLNPluginLayerProperty *tempResult = [[MLNPluginLayerProperty alloc] init];
  tempResult.propertyName = propertyName;
  tempResult.propertyType = propertyType;

  if (propertyType == MLNPluginLayerPropertyTypeSingleFloat) {
    if ([defaultValue isKindOfClass:[NSNumber class]]) {
      tempResult.singleFloatDefaultValue = [defaultValue floatValue];
    }
  } else if (propertyType == MLNPluginLayerPropertyTypeColor) {
#if TARGET_OS_IPHONE
    if ([defaultValue isKindOfClass:[UIColor class]]) {
#else
    if ([defaultValue isKindOfClass:[NSColor class]]) {
#endif
      tempResult.colorDefaultValue = defaultValue;
    }
  }

  return tempResult;
}

- (id)init {
  // Base class implemenation
  if (self = [super init]) {
    // Default setup
    self.propertyType = MLNPluginLayerPropertyTypeUnknown;
    self.propertyName = @"unknown";

    // Default values for the various types
    self.singleFloatDefaultValue = 1.0;
  }
  return self;
}

@end

@interface MLNPluginLayer () {
}

@property (nonatomic, copy, nullable) MLNPluginLayerTextureBindingCallback _textureBindingCallback;

@end

@implementation MLNPluginLayer

- (void)setTextureBindingCallback:(MLNPluginLayerTextureBindingCallback)callback {
  self._textureBindingCallback = callback;
}

// This is the layer type in the style that is used
+ (MLNPluginLayerCapabilities *)layerCapabilities {
  // Base class returns the class name just to return something
  // TODO: Add an assert/etc or something to notify the developer that this needs to be overridden
  return nil;
}

- (void)onRenderLayer:(MLNMapView *)mapView
        renderEncoder:(id<MTLRenderCommandEncoder>)renderEncoder {
  // Base class does nothing
}

- (void)onUpdateLayer:(MLNPluginLayerDrawingContext)drawingContext {
  // Base class does nothing
}

- (void)onUpdateLayerProperties:(NSDictionary *)layerProperties {
  // Base class does nothing
}

// If the layer properties indicate that this layer has a the ability to intercept
// features, then this method will be called when a feature is loaded
- (void)onFeatureCollectionLoaded:(MLNPluginLayerTileFeatureCollection *)tileFeatureCollection {
  // Base class does nothing
}

/// Called when a set of features are unloaded because the tile goes out of scene/etc
- (void)onFeatureCollectionUnloaded:(MLNPluginLayerTileFeatureCollection *)tileFeatureCollection {
  // Base class does nothing
}

- (NSArray *)onSpriteProperties {
  // Base class does nothing
  return nil;
};

- (NSArray *)onGlyphProperties {
  // Base class does nothing
  return nil;
};

- (NSArray *)onBaseFontStack {
  // Base class does nothing
  return nil;
};

/// Added to a map view
- (void)didMoveToMapView:(MLNMapView *)mapView {
  // Base class does nothing
}

- (void)bindTexture:(MLNTexture *)texture location:(int32_t)location {
  auto tex = [texture getTexture];
  self._textureBindingCallback(tex, location);
}

@end

@implementation MLNPluginLayerCapabilities
@end
