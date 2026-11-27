#import "MapLibre.h"

NS_ASSUME_NONNULL_BEGIN

/**
 EPSG:27700, the British National Grid from WGS84 through the Helmert transformation, on
 Ordnance Survey's tile grid: the projection the core tests, the render tests and the GLFW demo
 register, wrapped for the SDK.
 */
FOUNDATION_EXTERN MLNCoordinateReferenceSystem *MBXNationalGridProjection(void);

/**
 The `projection/national-grid/crosshairs` render test's style, named "National Grid crosshairs
 (EPSG:27700)" and drawn from the tiles in the bundle, written to a temporary file. Nil when the
 bundle doesn't have it.
 */
FOUNDATION_EXTERN NSURL *_Nullable MBXNationalGridCrosshairsStyleURL(NSBundle *bundle);

NS_ASSUME_NONNULL_END
