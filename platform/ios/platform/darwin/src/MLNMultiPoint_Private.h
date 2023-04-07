#import "MLNMultiPoint.h"

#import "MLNGeometry.h"

#import <mbgl/annotation/annotation.hpp>
#import <mbgl/util/feature.hpp>
#import <vector>

#import <CoreGraphics/CoreGraphics.h>
#import <CoreLocation/CoreLocation.h>

NS_ASSUME_NONNULL_BEGIN

@class MLNPolygon;
@class MLNPolyline;

@protocol MLNMultiPointDelegate;

@interface MLNMultiPoint (Private)

- (instancetype)initWithCoordinates:(const CLLocationCoordinate2D *)coords count:(NSUInteger)count;
- (BOOL)intersectsOverlayBounds:(MLNCoordinateBounds)overlayBounds;

/** Constructs a shape annotation object, asking the delegate for style values. */
- (mbgl::Annotation)annotationObjectWithDelegate:(id <MLNMultiPointDelegate>)delegate;

@end

/** An object that tells the MLNMultiPoint instance how to style itself. */
@protocol MLNMultiPointDelegate <NSObject>

/** Returns the fill alpha value for the given annotation. */
- (double)alphaForShapeAnnotation:(MLNShape *)annotation;

/** Returns the stroke color object for the given annotation. */
- (mbgl::Color)strokeColorForShapeAnnotation:(MLNShape *)annotation;

/** Returns the fill color object for the given annotation. */
- (mbgl::Color)fillColorForPolygonAnnotation:(MLNPolygon *)annotation;

/** Returns the stroke width object for the given annotation. */
- (CGFloat)lineWidthForPolylineAnnotation:(MLNPolyline *)annotation;

@end

NS_ASSUME_NONNULL_END
