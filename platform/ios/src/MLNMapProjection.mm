#include <mbgl/map/map.hpp>
#include <mbgl/map/map_projection.hpp>
#include <mbgl/util/geo.hpp>
#include <mbgl/util/projection.hpp>

#import "MLNMapProjection.h"
#import "MLNMapView_Private.h"
#import "MLNGeometry_Private.h"

@interface MLNMapProjection ()

@property (nonatomic) CGSize mapFrameSize;

@end

@implementation MLNMapProjection
{
    std::unique_ptr<mbgl::MapProjection> _mbglProjection;
}

- (instancetype)initWithMapView:(MLNMapView *)mapView
{
    if (self = [super init])
    {
        _mbglProjection = std::make_unique<mbgl::MapProjection>([mapView mbglMap]);
        self.mapFrameSize = mapView.frame.size;
    }
    return self;
}

- (MLNMapCamera*)camera
{
    mbgl::CameraOptions cameraOptions = _mbglProjection->getCamera();

    CLLocationCoordinate2D centerCoordinate = MLNLocationCoordinate2DFromLatLng(*cameraOptions.center);
    double zoomLevel = *cameraOptions.zoom;
    CLLocationDirection direction = mbgl::util::wrap(*cameraOptions.bearing, 0., 360.);
    CGFloat pitch = *cameraOptions.pitch;
    CLLocationDistance altitude = MLNAltitudeForZoomLevel(zoomLevel, pitch,
                                                          centerCoordinate.latitude, self.mapFrameSize);
    return [MLNMapCamera cameraLookingAtCenterCoordinate:centerCoordinate altitude:altitude
                                                   pitch:pitch heading:direction];
}

- (void)setCamera:(MLNMapCamera * _Nonnull)camera withEdgeInsets:(UIEdgeInsets)insets
{
    mbgl::CameraOptions cameraOptions;
    if (CLLocationCoordinate2DIsValid(camera.centerCoordinate))
    {
        cameraOptions.center = MLNLatLngFromLocationCoordinate2D(camera.centerCoordinate);
    }
    cameraOptions.padding = MLNEdgeInsetsFromNSEdgeInsets(insets);
    cameraOptions.zoom = MLNZoomLevelForAltitude(camera.altitude, camera.pitch,
                                                 camera.centerCoordinate.latitude,
                                                 self.mapFrameSize);
    if (camera.heading >= 0)
    {
        cameraOptions.bearing = camera.heading;
    }
    if (camera.pitch >= 0)
    {
        cameraOptions.pitch = camera.pitch;
    }

    _mbglProjection->setCamera(cameraOptions);
}

- (void)setVisibleCoordinateBounds:(MLNCoordinateBounds)bounds edgePadding:(UIEdgeInsets)insets {
    CLLocationCoordinate2D coordinates[] = {
        {bounds.ne.latitude, bounds.sw.longitude},
        bounds.sw,
        {bounds.sw.latitude, bounds.ne.longitude},
        bounds.ne,
    };

    mbgl::EdgeInsets padding = MLNEdgeInsetsFromNSEdgeInsets(insets);
    std::vector<mbgl::LatLng> latLngs;
    latLngs.reserve(4);
    for (NSUInteger i = 0; i < 4; i++)
    {
        latLngs.push_back({coordinates[i].latitude, coordinates[i].longitude});
    }

    _mbglProjection->setVisibleCoordinates(latLngs, padding);
}

- (CLLocationCoordinate2D)convertPoint:(CGPoint)point
{
    mbgl::ScreenCoordinate screenCoordinate = mbgl::ScreenCoordinate(point.x, point.y);
    return MLNLocationCoordinate2DFromLatLng(_mbglProjection->latLngForPixel(screenCoordinate).wrapped());
}

- (CGPoint)convertCoordinate:(CLLocationCoordinate2D)coordinate
{
    if ( !CLLocationCoordinate2DIsValid(coordinate))
    {
        return CGPointMake(NAN, NAN);
    }

    mbgl::LatLng latLng = MLNLatLngFromLocationCoordinate2D(coordinate);
    mbgl::ScreenCoordinate pixel = _mbglProjection->pixelForLatLng(latLng);
    return CGPointMake(pixel.x, pixel.y);
}

- (CLLocationDistance)metersPerPoint
{
    mbgl::CameraOptions cameraOptions = _mbglProjection->getCamera();
    return mbgl::Projection::getMetersPerPixelAtLatitude(cameraOptions.center->latitude(),
                                                         *cameraOptions.zoom);
}


@end
