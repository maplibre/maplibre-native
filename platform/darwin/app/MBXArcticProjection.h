#import "MapLibre.h"

/**
 EPSG:3413, NSIDC's north polar stereographic projection of the WGS 84 ellipsoid, true to scale at
 70 N with 45 W pointing down, on the grid NASA GIBS serves it in (tile 0/0/0 spans 8388608 m from
 x, y = -4194304, 4194304). Formulas from Snyder, Map Projections: A Working Manual (1987), 21-34
 and 21-38 to 21-41.
 */
FOUNDATION_EXTERN MLNCoordinateReferenceSystem *MBXArcticProjection(void);
