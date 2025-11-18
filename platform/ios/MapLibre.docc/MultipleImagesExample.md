# Adding Multiple Images

Adding images to the map and assigning them to POI types

> This example uses UIKit.

This example uses POIs in national parks and adds them to the map with data and icons provided by the National Park Service. Both the data and the icons are in the public domain.

## Preparation

We've done all the work for you and generated [`pois-nps.mbtiles`](https://maplibre-native.s3.eu-central-1.amazonaws.com/ios-swift-example-app-resources/pois-nps.mbtiles), which you can download directly.
You can skip the rest of this section unless you're curious how we built the MBTiles from the source data.

- Data, Shapefile download: [https://public-nps.opendata.arcgis.com/datasets/a40e2faa953b4c5cb7fe10004dc3008e_0/](https://public-nps.opendata.arcgis.com/datasets/a40e2faa953b4c5cb7fe10004dc3008e_0/)
- Icons: [https://www.nps.gov/subjects/gisandmapping/map-symbols-patterns-for-nps-maps.htm](https://www.nps.gov/subjects/gisandmapping/map-symbols-patterns-for-nps-maps.htm)

Convert the data first to GeoJSON format with [`ogr2ogr`](https://gdal.org/programs/ogr2ogr.html) and then to GeoJSON format using [tippecanoe](https://github.com/felt/tippecanoe).

```
ogr2ogr -f GeoJSON pois.json -t_srs EPSG:4326 nps-pois.shp
tippecanoe -o pois-nps.mbtiles pois.json
```

The resulting `.mbtiles` file can be hosted with a tile server such as [Martin](https://martin.maplibre.org/) or embedded in the app bundle as a resource. Since the file is quite small in this case we will use that last option in this example. Martin comes with a [`mbtiles` binary](https://maplibre.org/martin/mbtiles-meta.html) that allows us to inspect what the MBTiles file contains from the command line.

```
mbtiles meta-all pois-nps.mbtiles
```

The important thing to note is that there is a `pois` layer whose features include a `POITYPE` attribute. While we could add icons for all kinds of `POITYPE` values included in the dataset, we will only add icons for restrooms, trailheads and viewpoints, and leave the rest as an exercise to the reader.

The icons need to be extracted with a tool like [Inkscape](https://inkscape.org/) because the National Park Service includes all icons in a big vector file. This is outside the scope of this example.

## Adding Icons on Style Load

The `.mbtiles` file needs to be added to the assets of the app. When the style loads we can add a ``MLNVectorTileSource`` with as URL `mbtiles://\(Bundle.main.bundlePath)/pois-nps.mbtiles"`.

The images need to be added to an imageset so they can be loaded as an `UIImage` and added to the style with ``MLNStyle/setImage:forName:`` as is shown below in the example. Note that you should set ``MLNVectorStyleLayer/sourceLayerIdentifier`` to match the layer name in the MBTiles file. Lastly a [`match` expression](https://maplibre.org/maplibre-style-spec/expressions/#match) is used to select the correct image based on `POITYPE` attribute present in the feature.

<!-- include-example(MultipleImagesExample) -->

```swift
class MultipleImagesExample: UIViewController, MLNMapViewDelegate {
    var mapView: MLNMapView!

    override func viewDidLoad() {
        super.viewDidLoad()

        mapView = MLNMapView(frame: view.bounds, styleURL: AMERICANA_STYLE)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.tintColor = .darkGray

        let glacierPoint = CLLocationCoordinate2D(latitude: 37.72836462778902, longitude: -119.57352616838511)
        mapView.setCenter(glacierPoint, animated: false)
        mapView.zoomLevel = 12

        mapView.delegate = self
        view.addSubview(mapView)
    }

    // Wait until the style is loaded before modifying the map style.
    func mapView(_: MLNMapView, didFinishLoading style: MLNStyle) {
        let source = MLNVectorTileSource(identifier: "pois-nps", configurationURL: URL(string: "mbtiles://\(Bundle.main.bundlePath)/pois-nps.mbtiles")!)

        style.addSource(source)

        let imagesToAdd = [
            ["nps-restrooms", "restrooms"],
            ["nps-trailhead", "trailhead"],
            ["nps-viewpoint", "viewpoint"],
        ]

        for imageInfo in imagesToAdd {
            if let imageName = imageInfo.first, let imageKey = imageInfo.last {
                if let image = UIImage(named: imageName) {
                    style.setImage(image, forName: imageKey)
                } else {
                    print("Failed to load \(imageName)")
                    return
                }
            }
        }

        let imageLayer = MLNSymbolStyleLayer(identifier: "npc-poi-images", source: source)
        imageLayer.sourceLayerIdentifier = "pois"
        imageLayer.iconImageName = NSExpression(mglJSONObject: [
            "match", ["get", "POITYPE"],
            "Restroom", "restrooms",
            "Trailhead", "trailhead",
            "Viewpoint", "viewpoint",
            "",
        ])

        style.addLayer(imageLayer)
    }
}
```

![](MultipleImagesExample.png)
