# Blocking Gestures

Constrain the map to a certain area.

> Note: This example uses SwiftUI.

To constrain the map to a certain area, you need to implement the ``MLNMapViewDelegate/mapView:shouldChangeFromCamera:toCamera:`` method of ``MLNMapViewDelegate``. By returning a boolean you can either allow or disallow a camera change.

<!-- include-example(BlockingGesturesExample) -->

```swift
// Denver, Colorado
private let center = CLLocationCoordinate2D(latitude: 39.748947, longitude: -104.995882)

// Colorado’s bounds
private let colorado = MLNCoordinateBounds(
    sw: CLLocationCoordinate2D(latitude: 36.986207, longitude: -109.049896),
    ne: CLLocationCoordinate2D(latitude: 40.989329, longitude: -102.062592)
)

struct BlockingGesturesExample: UIViewRepresentable {
    class Coordinator: NSObject, MLNMapViewDelegate {
        func mapView(_ mapView: MLNMapView, shouldChangeFrom _: MLNMapCamera, to newCamera: MLNMapCamera) -> Bool {
            // Get the current camera to restore it after.
            let currentCamera = mapView.camera

            // From the new camera obtain the center to test if it’s inside the boundaries.
            let newCameraCenter = newCamera.centerCoordinate

            // Set the map’s visible bounds to newCamera.
            mapView.camera = newCamera
            let newVisibleCoordinates = mapView.visibleCoordinateBounds

            // Revert the camera.
            mapView.camera = currentCamera

            // Test if the newCameraCenter and newVisibleCoordinates are inside self.colorado.
            let inside = MLNCoordinateInCoordinateBounds(newCameraCenter, colorado)
            let intersects = MLNCoordinateInCoordinateBounds(newVisibleCoordinates.ne, colorado) && MLNCoordinateInCoordinateBounds(newVisibleCoordinates.sw, colorado)

            return inside && intersects
        }
    }

    func makeUIView(context: Context) -> MLNMapView {
        let mapView = MLNMapView(frame: .zero, styleURL: VERSATILES_COLORFUL_STYLE)
        mapView.setCenter(center, zoomLevel: 10, direction: 0, animated: false)
        mapView.delegate = context.coordinator

        return mapView
    }

    func updateUIView(_: MLNMapView, context _: Context) {}

    func makeCoordinator() -> Coordinator {
        Coordinator()
    }
}
```

The style used in this example can be found here: <doc:ExampleStyles>.
