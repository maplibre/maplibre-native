#import "MLNTileSource_Private.h"

#import "MLNAttributionInfo_Private.h"
#import "MLNGeometry_Private.h"
#import "MLNRasterDEMSource.h"
#import "MLNVectorTileSource.h"
#import "NSString+MLNAdditions.h"
#import "NSValue+MLNAdditions.h"

#if TARGET_OS_IPHONE
    #import <UIKit/UIKit.h>
#else
    #import <Cocoa/Cocoa.h>
#endif

#include <mbgl/util/tileset.hpp>

const MLNTileSourceOption MLNTileSourceOptionMinimumZoomLevel = @"MLNTileSourceOptionMinimumZoomLevel";
const MLNTileSourceOption MLNTileSourceOptionMaximumZoomLevel = @"MLNTileSourceOptionMaximumZoomLevel";
const MLNTileSourceOption MLNTileSourceOptionCoordinateBounds = @"MLNTileSourceOptionCoordinateBounds";
const MLNTileSourceOption MLNTileSourceOptionAttributionHTMLString = @"MLNTileSourceOptionAttributionHTMLString";
const MLNTileSourceOption MLNTileSourceOptionAttributionInfos = @"MLNTileSourceOptionAttributionInfos";
const MLNTileSourceOption MLNTileSourceOptionTileCoordinateSystem = @"MLNTileSourceOptionTileCoordinateSystem";

@implementation MLNTileSource

- (NSURL *)configurationURL {
    [NSException raise:MLNAbstractClassException
                format:@"MLNTileSource is an abstract class"];
    return nil;
}

- (NSArray<MLNAttributionInfo *> *)attributionInfos {
    return [self attributionInfosWithFontSize:0 linkColor:nil];
}

- (NSArray<MLNAttributionInfo *> *)attributionInfosWithFontSize:(CGFloat)fontSize linkColor:(nullable MLNColor *)linkColor {
    return [MLNAttributionInfo attributionInfosFromHTMLString:self.attributionHTMLString
                                                     fontSize:fontSize
                                                    linkColor:linkColor];
}

- (NSString *)attributionHTMLString {
    [NSException raise:MLNAbstractClassException
                format:@"MLNTileSource is an abstract class"];
    return nil;
}

@end

mbgl::Tileset MLNTileSetFromTileURLTemplates(NSArray<NSString *> *tileURLTemplates, NSDictionary<MLNTileSourceOption, id> * _Nullable options) {
    mbgl::Tileset tileSet;

    for (NSString *tileURLTemplate in tileURLTemplates) {
        tileSet.tiles.push_back(tileURLTemplate.UTF8String);
    }

    // set the minimum / maximum zoom range to the values specified by this class if they
    // were set. otherwise, use the core objects default values
    if (NSNumber *minimumZoomLevel = options[MLNTileSourceOptionMinimumZoomLevel]) {
        if (![minimumZoomLevel isKindOfClass:[NSNumber class]]) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNTileSourceOptionMinimumZoomLevel must be set to an NSNumber."];
        }
        tileSet.zoomRange.min = minimumZoomLevel.integerValue;
    }
    if (NSNumber *maximumZoomLevel = options[MLNTileSourceOptionMaximumZoomLevel]) {
        if (![maximumZoomLevel isKindOfClass:[NSNumber class]]) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNTileSourceOptionMinimumZoomLevel must be set to an NSNumber."];
        }
        tileSet.zoomRange.max = maximumZoomLevel.integerValue;
    }
    if (tileSet.zoomRange.min > tileSet.zoomRange.max) {
        [NSException raise:NSInvalidArgumentException
                    format:@"MLNTileSourceOptionMinimumZoomLevel must be less than MLNTileSourceOptionMaximumZoomLevel."];
    }

    if (NSValue *coordinateBounds = options[MLNTileSourceOptionCoordinateBounds]) {
        if (![coordinateBounds isKindOfClass:[NSValue class]]
            && strcmp(coordinateBounds.objCType, @encode(MLNCoordinateBounds)) == 0) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNTileSourceOptionCoordinateBounds must be set to an NSValue containing an MLNCoordinateBounds."];
        }
        tileSet.bounds = MLNLatLngBoundsFromCoordinateBounds(coordinateBounds.MLNCoordinateBoundsValue);
    }

    if (NSString *attribution = options[MLNTileSourceOptionAttributionHTMLString]) {
        if (![attribution isKindOfClass:[NSString class]]) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNTileSourceOptionAttributionHTMLString must be set to a string."];
        }
        tileSet.attribution = attribution.UTF8String;
    }

    if (NSArray *attributionInfos = options[MLNTileSourceOptionAttributionInfos]) {
        if (![attributionInfos isKindOfClass:[NSArray class]]) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNTileSourceOptionAttributionInfos must be set to a string."];
        }

        NSAttributedString *attributedString = [MLNAttributionInfo attributedStringForAttributionInfos:attributionInfos];
#if TARGET_OS_IPHONE
        static NSString * const NSExcludedElementsDocumentAttribute = @"ExcludedElements";
#endif
        NSDictionary *documentAttributes = @{
            NSDocumentTypeDocumentAttribute: NSHTMLTextDocumentType,
            NSCharacterEncodingDocumentAttribute: @(NSUTF8StringEncoding),
            // The attribution string is meant to be a simple, inline fragment, not a full-fledged, validating document.
            NSExcludedElementsDocumentAttribute: @[@"XML", @"DOCTYPE", @"html", @"head", @"meta", @"title", @"style", @"body", @"p"],
        };
        NSData *data = [attributedString dataFromRange:attributedString.mgl_wholeRange documentAttributes:documentAttributes error:NULL];
        NSString *html = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        tileSet.attribution = html.UTF8String;
    }

    if (NSNumber *tileCoordinateSystemNumber = options[MLNTileSourceOptionTileCoordinateSystem]) {
        if (![tileCoordinateSystemNumber isKindOfClass:[NSValue class]]) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNTileSourceOptionTileCoordinateSystem must be set to an NSValue or NSNumber."];
        }
        MLNTileCoordinateSystem tileCoordinateSystem;
        [tileCoordinateSystemNumber getValue:&tileCoordinateSystem];
        switch (tileCoordinateSystem) {
            case MLNTileCoordinateSystemXYZ:
                tileSet.scheme = mbgl::Tileset::Scheme::XYZ;
                break;
            case MLNTileCoordinateSystemTMS:
                tileSet.scheme = mbgl::Tileset::Scheme::TMS;
                break;
        }
    }

    NSNumber *encodingNumber = options[MLNTileSourceOptionDEMEncoding];
    if (encodingNumber) {
        if (![encodingNumber isKindOfClass:[NSValue class]]) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNTileSourceOptionDEMEncoding must be set to an NSValue or NSNumber."];
        }
        MLNDEMEncoding encoding;
        [encodingNumber getValue:&encoding];
        switch (encoding) {
            case MLNDEMEncodingMapbox:
                tileSet.rasterEncoding = mbgl::Tileset::RasterEncoding::Mapbox;
                break;
            case MLNDEMEncodingTerrarium:
                tileSet.rasterEncoding = mbgl::Tileset::RasterEncoding::Terrarium;
                break;
        }
    }

    encodingNumber = options[MLNVectorTileSourceOptionEncoding];
    if (encodingNumber) {
        if (![encodingNumber isKindOfClass:[NSValue class]]) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNTileSourceOptionVectorEncoding must be set to an NSValue or NSNumber."];
        }
        MLNVectorTileSourceEncoding encoding;
        [encodingNumber getValue:&encoding];
        switch (encoding) {
            case MLNVectorTileSourceEncodingMapbox:
                tileSet.vectorEncoding = mbgl::Tileset::VectorEncoding::Mapbox;
                break;
            case MLNVectorTileSourceEncodingMLT:
                tileSet.vectorEncoding = mbgl::Tileset::VectorEncoding::MLT;
                break;
        }
    }

    return tileSet;
}
