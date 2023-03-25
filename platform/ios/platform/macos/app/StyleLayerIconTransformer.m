#import "StyleLayerIconTransformer.h"

#import <Mapbox/Mapbox.h>

@implementation StyleLayerIconTransformer

+ (Class)transformedValueClass {
    return [NSString class];
}

+ (BOOL)allowsReverseTransformation {
    return NO;
}

- (id)transformedValue:(MLNStyleLayer *)layer {
    if ([layer isKindOfClass:[MLNBackgroundStyleLayer class]]) {
        return [NSImage imageNamed:@"background"];
    }
    if ([layer isKindOfClass:[MLNCircleStyleLayer class]]) {
        return [NSImage imageNamed:@"circle"];
    }
    if ([layer isKindOfClass:[MLNFillStyleLayer class]]) {
        return [NSImage imageNamed:@"fill"];
    }
    if ([layer isKindOfClass:[MLNFillExtrusionStyleLayer class]]) {
        return [NSImage imageNamed:@"fill-extrusion"];
    }
    if ([layer isKindOfClass:[MLNLineStyleLayer class]]) {
        return [NSImage imageNamed:@"NSListViewTemplate"];
    }
    if ([layer isKindOfClass:[MLNRasterStyleLayer class]]) {
        return [[NSWorkspace sharedWorkspace] iconForFileType:@"jpg"];
    }
    if ([layer isKindOfClass:[MLNSymbolStyleLayer class]]) {
        return [NSImage imageNamed:@"symbol"];
    }
    if ([layer isKindOfClass:[MLNHeatmapStyleLayer class]]) {
        return [NSImage imageNamed:@"heatmap"];
    }
    if ([layer isKindOfClass:[MLNHillshadeStyleLayer class]]) {
        return [NSImage imageNamed:@"hillshade"];
    }
    
    return nil;
}

@end
