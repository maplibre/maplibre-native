import XCTest
import MapLibre

class MLNMapViewDelegateIntegrationTests: XCTestCase {

    func testCoverage() {
        MLNSDKTestHelpers.checkTestsContainAllMethods(testClass: MLNMapViewDelegateIntegrationTests.self, in: MLNMapViewDelegate.self)
    }

}

extension MLNMapViewDelegateIntegrationTests: MLNMapViewDelegate {

    func mapViewRegionIsChanging(_ mapView: MLNMapView) {}

    func mapViewRegionIsChanging(_ mapView: MLNMapView, reason: MLNCameraChangeReason) {}

    func mapView(_ mapView: MLNMapView, regionIsChangingWith reason: MLNCameraChangeReason) {}

    func mapView(_ mapView: MLNMapView, didChange mode: MLNUserTrackingMode, animated: Bool) {}

    func mapViewDidFinishLoadingMap(_ mapView: MLNMapView) {}

    func mapViewDidStopLocatingUser(_ mapView: MLNMapView) {}

    func mapViewWillStartLoadingMap(_ mapView: MLNMapView) {}

    func mapViewWillStartLocatingUser(_ mapView: MLNMapView) {}

    func mapViewWillStartRenderingMap(_ mapView: MLNMapView) {}

    func mapViewWillStartRenderingFrame(_ mapView: MLNMapView) {}

    func mapView(_ mapView: MLNMapView, didFinishLoading style: MLNStyle) {}

    func mapView(_ mapView: MLNMapView, didSelect annotation: MLNAnnotation) {}

    func mapView(_ mapView: MLNMapView, didDeselect annotation: MLNAnnotation) {}

    func mapView(_ mapView: MLNMapView, didSingleTapAt coordinate: CLLocationCoordinate2D) {}

    func mapView(_ mapView: MLNMapView, regionDidChangeAnimated animated: Bool) {}

    func mapView(_ mapView: MLNMapView, regionDidChangeWith reason: MLNCameraChangeReason, animated: Bool) {}

    func mapView(_ mapView: MLNMapView, regionWillChangeAnimated animated: Bool) {}

    func mapView(_ mapView: MLNMapView, regionWillChangeWith reason: MLNCameraChangeReason, animated: Bool) {}

    func mapViewDidFailLoadingMap(_ mapView: MLNMapView, withError error: Error) {}

    func mapView(_ mapView: MLNMapView, didUpdate userLocation: MLNUserLocation?) {}

    func mapViewDidFinishRenderingMap(_ mapView: MLNMapView, fullyRendered: Bool) {}
    
     func mapViewDidBecomeIdle(_ mapView: MLNMapView) {}

    func mapView(_ mapView: MLNMapView, didFailToLocateUserWithError error: Error) {}

    func mapView(_ mapView: MLNMapView, tapOnCalloutFor annotation: MLNAnnotation) {}

    func mapViewDidFinishRenderingFrame(_ mapView: MLNMapView, fullyRendered: Bool) {}

    func mapViewDidFinishRenderingFrame(_ mapView: MLNMapView, fullyRendered: Bool, frameTime: Double) {}
    
    func mapView(_ mapView: MLNMapView, shapeAnnotationIsEnabled annotation: MLNShape) -> Bool { return false }

    func mapView(_ mapView: MLNMapView, didAdd annotationViews: [MLNAnnotationView]) {}

    func mapView(_ mapView: MLNMapView, didSelect annotationView: MLNAnnotationView) {}

    func mapView(_ mapView: MLNMapView, didDeselect annotationView: MLNAnnotationView) {}

    func mapView(_ mapView: MLNMapView, alphaForShapeAnnotation annotation: MLNShape) -> CGFloat { return 0 }

    func mapView(_ mapView: MLNMapView, viewFor annotation: MLNAnnotation) -> MLNAnnotationView? { return nil }

    func mapView(_ mapView: MLNMapView, imageFor annotation: MLNAnnotation) -> MLNAnnotationImage? { return nil }

    func mapView(_ mapView: MLNMapView, annotationCanShowCallout annotation: MLNAnnotation) -> Bool { return false }

    func mapView(_ mapView: MLNMapView, calloutViewFor annotation: MLNAnnotation) -> MLNCalloutView? { return nil }

    func mapView(_ mapView: MLNMapView, strokeColorForShapeAnnotation annotation: MLNShape) -> UIColor { return .black }

    func mapView(_ mapView: MLNMapView, fillColorForPolygonAnnotation annotation: MLNPolygon) -> UIColor { return .black }

    func mapView(_ mapView: MLNMapView, leftCalloutAccessoryViewFor annotation: MLNAnnotation) -> UIView? { return nil }

    func mapView(_ mapView: MLNMapView, lineWidthForPolylineAnnotation annotation: MLNPolyline) -> CGFloat { return 0 }

    func mapView(_ mapView: MLNMapView, rightCalloutAccessoryViewFor annotation: MLNAnnotation) -> UIView? { return nil }

    func mapView(_ mapView: MLNMapView, annotation: MLNAnnotation, calloutAccessoryControlTapped control: UIControl) {}

    func mapView(_ mapView: MLNMapView, shouldChangeFrom oldCamera: MLNMapCamera, to newCamera: MLNMapCamera) -> Bool { return false }

    func mapView(_ mapView: MLNMapView, shouldChangeFrom oldCamera: MLNMapCamera, to newCamera: MLNMapCamera, reason: MLNCameraChangeReason) -> Bool { return false }
    
    func mapViewUserLocationAnchorPoint(_ mapView: MLNMapView) -> CGPoint { return CGPoint(x: 100, y: 100) }
    
    func mapView(_ mapView: MLNMapView, didFailToLoadImage imageName: String) -> UIImage? { return nil }
    
    func mapView(_ mapView: MLNMapView, shouldRemoveStyleImage imageName: String) -> Bool { return false }

    func mapView(_ mapView: MLNMapView, didChangeLocationManagerAuthorization manager: MLNLocationManager) {}

    func mapView(styleForDefaultUserLocationAnnotationView mapView: MLNMapView) -> MLNUserLocationAnnotationViewStyle { return MLNUserLocationAnnotationViewStyle() }
}
