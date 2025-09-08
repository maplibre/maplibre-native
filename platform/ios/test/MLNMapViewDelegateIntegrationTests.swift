import MapLibre
import XCTest

class MLNMapViewDelegateIntegrationTests: XCTestCase {
    func testCoverage() {
        MLNSDKTestHelpers.checkTestsContainAllMethods(testClass: MLNMapViewDelegateIntegrationTests.self, in: MLNMapViewDelegate.self)
    }
}

extension MLNMapViewDelegateIntegrationTests: MLNMapViewDelegate {
    func mapViewRegionIsChanging(_: MLNMapView) {}

    func mapViewRegionIsChanging(_: MLNMapView, reason _: MLNCameraChangeReason) {}

    func mapView(_: MLNMapView, regionIsChangingWith _: MLNCameraChangeReason) {}

    func mapView(_: MLNMapView, didChange _: MLNUserTrackingMode, animated _: Bool) {}

    func mapViewDidFinishLoadingMap(_: MLNMapView) {}

    func mapViewDidStopLocatingUser(_: MLNMapView) {}

    func mapViewWillStartLoadingMap(_: MLNMapView) {}

    func mapViewWillStartLocatingUser(_: MLNMapView) {}

    func mapViewWillStartRenderingMap(_: MLNMapView) {}

    func mapViewWillStartRenderingFrame(_: MLNMapView) {}

    func mapView(_: MLNMapView, didFinishLoading _: MLNStyle) {}

    func mapView(_: MLNMapView, sourceDidChange _: MLNSource) {}

    func mapView(_: MLNMapView, didSelect _: MLNAnnotation) {}

    func mapView(_: MLNMapView, didDeselect _: MLNAnnotation) {}

    func mapView(_: MLNMapView, didSingleTapAt _: CLLocationCoordinate2D) {}

    func mapView(_: MLNMapView, regionDidChangeAnimated _: Bool) {}

    func mapView(_: MLNMapView, regionDidChangeWith _: MLNCameraChangeReason, animated _: Bool) {}

    func mapView(_: MLNMapView, regionWillChangeAnimated _: Bool) {}

    func mapView(_: MLNMapView, regionWillChangeWith _: MLNCameraChangeReason, animated _: Bool) {}

    func mapViewDidFailLoadingMap(_: MLNMapView, withError _: Error) {}

    func mapView(_: MLNMapView, didUpdate _: MLNUserLocation?) {}

    func mapViewDidFinishRenderingMap(_: MLNMapView, fullyRendered _: Bool) {}

    func mapViewDidBecomeIdle(_: MLNMapView) {}

    func mapView(_: MLNMapView, didFailToLocateUserWithError _: Error) {}

    func mapView(_: MLNMapView, tapOnCalloutFor _: MLNAnnotation) {}

    func mapViewDidFinishRenderingFrame(_: MLNMapView, fullyRendered _: Bool) {}

    func mapViewDidFinishRenderingFrame(_: MLNMapView, fullyRendered _: Bool, frameEncodingTime _: Double, frameRenderingTime _: Double) {}

    func mapViewDidFinishRenderingFrame(_: MLNMapView, fullyRendered _: Bool, renderingStats _: MLNRenderingStats) {}

    func mapView(_: MLNMapView, shapeAnnotationIsEnabled _: MLNShape) -> Bool { false }

    func mapView(_: MLNMapView, didAdd _: [MLNAnnotationView]) {}

    func mapView(_: MLNMapView, didSelect _: MLNAnnotationView) {}

    func mapView(_: MLNMapView, didDeselect _: MLNAnnotationView) {}

    func mapView(_: MLNMapView, alphaForShapeAnnotation _: MLNShape) -> CGFloat { 0 }

    func mapView(_: MLNMapView, viewFor _: MLNAnnotation) -> MLNAnnotationView? { nil }

    func mapView(_: MLNMapView, imageFor _: MLNAnnotation) -> MLNAnnotationImage? { nil }

    func mapView(_: MLNMapView, annotationCanShowCallout _: MLNAnnotation) -> Bool { false }

    func mapView(_: MLNMapView, calloutViewFor _: MLNAnnotation) -> MLNCalloutView? { nil }

    func mapView(_: MLNMapView, strokeColorForShapeAnnotation _: MLNShape) -> UIColor { .black }

    func mapView(_: MLNMapView, fillColorForPolygonAnnotation _: MLNPolygon) -> UIColor { .black }

    func mapView(_: MLNMapView, leftCalloutAccessoryViewFor _: MLNAnnotation) -> UIView? { nil }

    func mapView(_: MLNMapView, lineWidthForPolylineAnnotation _: MLNPolyline) -> CGFloat { 0 }

    func mapView(_: MLNMapView, rightCalloutAccessoryViewFor _: MLNAnnotation) -> UIView? { nil }

    func mapView(_: MLNMapView, annotation _: MLNAnnotation, calloutAccessoryControlTapped _: UIControl) {}

    func mapView(_: MLNMapView, shouldChangeFrom _: MLNMapCamera, to _: MLNMapCamera) -> Bool { false }

    func mapView(_: MLNMapView, shouldChangeFrom _: MLNMapCamera, to _: MLNMapCamera, reason _: MLNCameraChangeReason) -> Bool { false }

    func mapViewUserLocationAnchorPoint(_: MLNMapView) -> CGPoint { CGPoint(x: 100, y: 100) }

    func mapView(_: MLNMapView, didFailToLoadImage _: String) -> UIImage? { nil }

    func mapView(_: MLNMapView, shouldRemoveStyleImage _: String) -> Bool { false }

    func mapView(_: MLNMapView, didChangeLocationManagerAuthorization _: MLNLocationManager) {}

    func mapView(styleForDefaultUserLocationAnnotationView _: MLNMapView) -> MLNUserLocationAnnotationViewStyle { MLNUserLocationAnnotationViewStyle() }

    func mapView(_: MLNMapView, shaderWillCompile _: Int, backend _: Int, defines _: String) {}
    func mapView(_: MLNMapView, shaderDidCompile _: Int, backend _: Int, defines _: String) {}
    func mapView(_: MLNMapView, shaderDidFailCompile _: Int, backend _: Int, defines _: String) {}

    func mapView(_: MLNMapView, glyphsWillLoad _: [String], range _: NSRange) {}
    func mapView(_: MLNMapView, glyphsDidLoad _: [String], range _: NSRange) {}
    func mapView(_: MLNMapView, glyphsDidError _: [String], range _: NSRange) {}

    func mapView(_: MLNMapView, tileDidTriggerAction _: MLNTileOperation,
                 x _: Int,
                 y _: Int,
                 z _: Int,
                 wrap _: Int,
                 overscaledZ _: Int,
                 sourceID _: String) {}

    func mapView(_: MLNMapView, spriteWillLoad _: String, url _: String) {}
    func mapView(_: MLNMapView, spriteDidLoad _: String, url _: String) {}
    func mapView(_: MLNMapView, spriteDidError _: String, url _: String) {}
}
