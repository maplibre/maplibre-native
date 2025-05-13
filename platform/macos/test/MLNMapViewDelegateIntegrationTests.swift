import Mapbox
import XCTest

class MLNMapViewDelegateIntegrationTests: XCTestCase {
    func testCoverage() {
        MLNSDKTestHelpers.checkTestsContainAllMethods(testClass: MLNMapViewDelegateIntegrationTests.self, in: MLNMapViewDelegate.self)
    }
}

extension MLNMapViewDelegateIntegrationTests: MLNMapViewDelegate {
    func mapView(_: MLNMapView, shouldChangeFrom _: MLNMapCamera, to _: MLNMapCamera) -> Bool { false }

    func mapView(_: MLNMapView, lineWidthForPolylineAnnotation _: MLNPolyline) -> CGFloat { 0 }

    func mapView(_: MLNMapView, annotationCanShowCallout _: MLNAnnotation) -> Bool { false }

    func mapView(_: MLNMapView, imageFor _: MLNAnnotation) -> MLNAnnotationImage? { nil }

    func mapView(_: MLNMapView, alphaForShapeAnnotation _: MLNShape) -> CGFloat { 0 }

    func mapViewDidFinishRenderingFrame(_: MLNMapView, fullyRendered _: Bool) {}

    func mapViewDidFinishRenderingFrame(_: MLNMapView, fullyRendered _: Bool, frameTime _: Double) {}

    func mapViewDidFinishRenderingMap(_: MLNMapView, fullyRendered _: Bool) {}

    func mapViewDidBecomeIdle(_: MLNMapView) {}

    func mapViewDidFailLoadingMap(_: MLNMapView, withError _: Error) {}

    func mapView(_: MLNMapView, didFailToLoadImage _: String) -> NSImage? { nil }

    func mapView(_: MLNMapView, shapeAnnotationIsEnabled _: MLNShape) -> Bool { false }

    func mapView(_: MLNMapView, didDeselect _: MLNAnnotation) {}

    func mapView(_: MLNMapView, didSelect _: MLNAnnotation) {}

    func mapView(_: MLNMapView, didFinishLoading _: MLNStyle) {}

    func mapView(_: MLNMapView, sourceDidChange _: MLNSource) {}

    func mapViewWillStartRenderingFrame(_: MLNMapView) {}

    func mapViewWillStartRenderingMap(_: MLNMapView) {}

    func mapViewWillStartLoadingMap(_: MLNMapView) {}

    func mapViewDidFinishLoadingMap(_: MLNMapView) {}

    func mapViewCameraIsChanging(_: MLNMapView) {}

    func mapView(_: MLNMapView, cameraDidChangeAnimated _: Bool) {}

    func mapView(_: MLNMapView, cameraWillChangeAnimated _: Bool) {}

    func mapView(_: MLNMapView, strokeColorForShapeAnnotation _: MLNShape) -> NSColor { .black }

    func mapView(_: MLNMapView, fillColorForPolygonAnnotation _: MLNPolygon) -> NSColor { .black }

    func mapView(_: MLNMapView, calloutViewControllerFor _: MLNAnnotation) -> NSViewController? { nil }

    func mapView(_: MLNMapView, shouldRemoveStyleImage _: String) -> Bool { false }
}
