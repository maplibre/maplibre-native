# Vector Tile Sources

Add and style a vector tile source

> This example uses UIKit

This example shows how a vector data source can be added and a style for it can be configured dynamically.

The tiles are [tiles around Innsbruck, Austria](https://github.com/maplibre/demotiles/tree/gh-pages/tiles-omt) that use the OpenMapTiles schema. We are interested in the [POIs](https://openmaptiles.org/schema/#poi) that are in the `poi` layer, and filter this further with an `NSPredicate` to only show POIs with a `class` of shop. Each POI has a `rank` which is normally used to reduce label density. In this example, we use it to demonstrate how a numeric attribute can be used for styling with the [step](https://maplibre.org/maplibre-style-spec/expressions/#step) expression. POIs with a rank between 0 and 10 get a red color, between 10 and 20 green, etc.

<!-- include-example(DDSCircleLayerExample) -->

```swift
class DDSCircleLayerExample: UIViewController, MLNMapViewDelegate {
    var mapView: MLNMapView!

    override func viewDidLoad() {
        super.viewDidLoad()

        mapView = MLNMapView(frame: view.bounds, styleURL: AMERICANA_STYLE)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.tintColor = .darkGray

        // Set the map’s center coordinate and zoom level.
        let innsbruck = CLLocationCoordinate2D(latitude: 47.26497, longitude: 11.4088)
        mapView.setCenter(innsbruck, animated: false)
        mapView.zoomLevel = 14

        mapView.delegate = self
        view.addSubview(mapView)
    }

    // Wait until the style is loaded before modifying the map style.
    func mapView(_: MLNMapView, didFinishLoading style: MLNStyle) {
        let source = MLNVectorTileSource(identifier: "demotiles", configurationURL: URL(string: "https://demotiles.maplibre.org/tiles-omt/tiles.json")!)

        style.addSource(source)

        let layer = MLNCircleStyleLayer(identifier: "poi-shop-style", source: source)

        layer.sourceLayerIdentifier = "poi"
        layer.predicate = NSPredicate(format: "class == %@", "shop")

        // Style the circle layer color based on the rank
        layer.circleColor = NSExpression(mglJSONObject: ["step", ["get", "rank"], 0, "red", 10, "green", 20, "blue", 30, "purple", 40, "yellow"] as [Any])
        layer.circleRadius = NSExpression(forConstantValue: 3)

        style.addLayer(layer)
    }
}
```

![](DDSCircleLayerExample.png)

Map data from OpenStreetMap. Style uses OpenMapTiles.
