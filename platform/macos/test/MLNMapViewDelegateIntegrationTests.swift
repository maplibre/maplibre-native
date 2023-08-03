import XCTest
import Mapbox

class MLNMapViewDelegateIntegrationTests: XCTestCase {

    func testCoverage() {
        MLNSDKTestHelpers.checkTestsContainAllMethods(testClass: MLNMapViewDelegateIntegrationTests.self, in: MLNMapViewDelegate.self)
    }

}

extension MLNMapViewDelegateIntegrationTests: MLNMapViewDelegate {

    func mapView(_ mapView: MLNMapView, shouldChangeFrom oldCamera: MLNMapCamera, to newCamera: MLNMapCamera) -> Bool { return false }

    func mapView(_ mapView: MLNMapView, lineWidthForPolylineAnnotation annotation: MLNPolyline) -> CGFloat { return 0 }

    func mapView(_ mapView: MLNMapView, annotationCanShowCallout annotation: MLNAnnotation) -> Bool { return false }

    func mapView(_ mapView: MLNMapView, imageFor annotation: MLNAnnotation) -> MLNAnnotationImage? { return nil }

    func mapView(_ mapView: MLNMapView, alphaForShapeAnnotation annotation: MLNShape) -> CGFloat { return 0 }

    func mapViewDidFinishRenderingFrame(_ mapView: MLNMapView, fullyRendered: Bool) {}

    func mapViewDidFinishRenderingMap(_ mapView: MLNMapView, fullyRendered: Bool) {}
    
    func mapViewDidBecomeIdle(_ mapView: MLNMapView) {}

    func mapViewDidFailLoadingMap(_ mapView: MLNMapView, withError error: Error) {}

    func mapView(_ mapView: MLNMapView, didFailToLoadImage imageName: String) -> NSImage? { return nil }
    
    func mapView(_ mapView: MLNMapView, shapeAnnotationIsEnabled annotation: MLNShape) -> Bool { return false }

    func mapView(_ mapView: MLNMapView, didDeselect annotation: MLNAnnotation) {}

    func mapView(_ mapView: MLNMapView, didSelect annotation: MLNAnnotation) {}

    func mapView(_ mapView: MLNMapView, didFinishLoading style: MLNStyle) {}

    func mapViewWillStartRenderingFrame(_ mapView: MLNMapView) {}

    func mapViewWillStartRenderingMap(_ mapView: MLNMapView) {}

    func mapViewWillStartLoadingMap(_ mapView: MLNMapView) {}

    func mapViewDidFinishLoadingMap(_ mapView: MLNMapView) {}

    func mapViewCameraIsChanging(_ mapView: MLNMapView) {}

    func mapView(_ mapView: MLNMapView, cameraDidChangeAnimated animated: Bool) {}

    func mapView(_ mapView: MLNMapView, cameraWillChangeAnimated animated: Bool) {}

    func mapView(_ mapView: MLNMapView, strokeColorForShapeAnnotation annotation: MLNShape) -> NSColor { return .black }

    func mapView(_ mapView: MLNMapView, fillColorForPolygonAnnotation annotation: MLNPolygon) -> NSColor { return .black }

    func mapView(_ mapView: MLNMapView, calloutViewControllerFor annotation: MLNAnnotation) -> NSViewController? { return nil }

    func mapView(_ mapView: MLNMapView, shouldRemoveStyleImage imageName: String) -> Bool { return false }
}
