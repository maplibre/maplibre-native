#import "Mapbox.h"

#import "MLNFoundation.h"
#import "MLNStyleValue.h"
#import "MLNStyleLayer.h"
#import "MLNGeometry.h"

@interface ExampleCustomDrawableStyleLayer : MLNCustomDrawableStyleLayer

- (instancetype)initWithIdentifier:(NSString *)identifier;

@end
