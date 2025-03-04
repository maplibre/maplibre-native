# Add Marker

Simply add a marker to a map!

> Note: This example uses UIKit.

<!-- include-example(AddMarkerSymbolExample) -->

```swift
class AddMarkerSymbolExampleUIKit: UIViewController, MLNMapViewDelegate {
    override func viewDidLoad() {
        super.viewDidLoad()

        let mapView = MLNMapView(frame: view.bounds)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.delegate = self

        // Set the map’s center coordinate and zoom level.
        mapView.setCenter(CLLocationCoordinate2D(latitude: 41.8864, longitude: -87.7135), zoomLevel: 13, animated: false)
        view.addSubview(mapView)
    }

    func mapView(_ mapView: MLNMapView, didFinishLoading style: MLNStyle) {
        // Create point to represent where the symbol should be placed
        let point = MLNPointAnnotation()
        point.coordinate = mapView.centerCoordinate

        // Create a data source to hold the point data
        let shapeSource = MLNShapeSource(identifier: "marker-source", shape: point, options: nil)

        // Create a style layer for the symbol
        let shapeLayer = MLNSymbolStyleLayer(identifier: "marker-style", source: shapeSource)

        // Add the image to the style's sprite
        if let image = UIImage(named: "house-icon") {
            style.setImage(image, forName: "home-symbol")
        }

        // Tell the layer to use the image in the sprite
        shapeLayer.iconImageName = NSExpression(forConstantValue: "home-symbol")

        // Add the source and style layer to the map
        style.addSource(shapeSource)
        style.addLayer(shapeLayer)
    }
}
```
