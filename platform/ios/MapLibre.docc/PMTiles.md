# PMTiles

Working with PMTiles

Starting MapLibre iOS 6.10.0, [PMTiles](https://docs.protomaps.com/pmtiles/) archives are supported as tile sources. Prefix any tile source URL with `pmtiles://` to read from a PMTiles archive:

- `pmtiles://https://` — stream tiles from a remote file
- `pmtiles://file://` — read a file from the device filesystem, including the app bundle

The `pmtiles://` prefix works with any [tile source type](https://maplibre.org/maplibre-style-spec/sources/) (e.g. `vector`, `raster`, `raster-dem`, etc.), from a style JSON or dynamically.

> Note: PMTiles sources do not support offline pack downloads or caching.

## Loading a style that uses PMTiles sources

Load a style JSON that references `pmtiles://` sources. The example below loads a satellite imagery basemap of the Central Alps and sets the initial camera position:

<!-- include-example(PMTilesStyleURL) -->

```swift
class PMTilesStyleURL: UIViewController {
    override func viewDidLoad() {
        super.viewDidLoad()
        let mapView = MLNMapView(
            frame: view.bounds,
            styleURL: URL(string: "https://demotiles.maplibre.org/pmtiles/raster/style-imagery.json")!
        )
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.setCenter(CLLocationCoordinate2D(latitude: 47.5, longitude: 12.0), zoomLevel: 9, animated: false)
        view.addSubview(mapView)
    }
}
```

## Adding a PMTiles source directly

Add `vector` sources from PMTiles archives to an already-loaded style. The example overlays two places datasets — [Overture Maps](https://overturemaps.org/) (yellow) and [Foursquare OS Places](https://opensource.foursquare.com/os-places/) (blue) — both using `MLNCircleStyleLayer`. The `sourceLayerIdentifier` names the layer within the archive to render:

<!-- include-example(PMTilesAddSource) -->

```swift
class PMTilesAddSource: UIViewController, MLNMapViewDelegate {
    var mapView: MLNMapView!

    override func viewDidLoad() {
        super.viewDidLoad()
        mapView = MLNMapView(frame: view.bounds)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.delegate = self
        view.addSubview(mapView)
    }

    func mapView(_: MLNMapView, didFinishLoading style: MLNStyle) {
        let overtureURL = URL(string: "pmtiles://https://overturemaps-tiles-us-west-2-beta.s3.amazonaws.com/2026-01-21/places.pmtiles")!
        let overtureSource = MLNVectorTileSource(identifier: "overture-places", configurationURL: overtureURL)
        style.addSource(overtureSource)
        let overtureLayer = MLNCircleStyleLayer(identifier: "overture-layer", source: overtureSource)
        overtureLayer.sourceLayerIdentifier = "place"
        overtureLayer.circleColor = NSExpression(forConstantValue: UIColor(red: 0.96, green: 0.78, blue: 0.0, alpha: 1))
        overtureLayer.circleRadius = NSExpression(forConstantValue: 4)
        overtureLayer.circleOpacity = NSExpression(forConstantValue: 0.6)
        style.addLayer(overtureLayer)

        let foursquareURL = URL(string: "pmtiles://https://oliverwipfli.ch/data/foursquare-os-places-10M-2024-11-20.pmtiles")!
        let foursquareSource = MLNVectorTileSource(identifier: "foursquare-places", configurationURL: foursquareURL)
        style.addSource(foursquareSource)
        let foursquareLayer = MLNCircleStyleLayer(identifier: "foursquare-layer", source: foursquareSource)
        foursquareLayer.sourceLayerIdentifier = "place"
        foursquareLayer.circleColor = NSExpression(forConstantValue: UIColor(red: 0.18, green: 0.85, blue: 1.0, alpha: 1))
        foursquareLayer.circleRadius = NSExpression(forConstantValue: 4)
        foursquareLayer.circleOpacity = NSExpression(forConstantValue: 0.6)
        style.addLayer(foursquareLayer)
    }
}
```

The same pattern works for `raster` archives using `MLNRasterTileSource` and `MLNRasterStyleLayer`:

<!-- include-example(PMTilesRasterSource) -->

```swift
class PMTilesRasterSource: UIViewController, MLNMapViewDelegate {
    var mapView: MLNMapView!

    override func viewDidLoad() {
        super.viewDidLoad()
        mapView = MLNMapView(frame: view.bounds)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.delegate = self
        view.addSubview(mapView)
    }

    func mapView(_: MLNMapView, didFinishLoading style: MLNStyle) {
        let url = URL(string: "pmtiles://https://demotiles.maplibre.org/pmtiles/raster/imagery.pmtiles")!
        let source = MLNRasterTileSource(identifier: "imagery", configurationURL: url, tileSize: 256)
        style.addSource(source)
        let layer = MLNRasterStyleLayer(identifier: "imagery", source: source)
        style.addLayer(layer)
    }
}
```

> Note: `raster-dem` sources added programmatically cannot specify `encoding` via the current iOS API. Define `raster-dem` sources with terrarium encoding in the style JSON instead, where the `encoding` field is fully supported.

## Loading a local PMTiles file

Bundle a `.pmtiles` file in your app target and reference it using its bundle URL. iOS app bundle files are accessible via `file://`, so no special handling is needed:

<!-- include-example(PMTilesLocalFile) -->

```swift
class PMTilesLocalFile: UIViewController, MLNMapViewDelegate {
    var mapView: MLNMapView!

    override func viewDidLoad() {
        super.viewDidLoad()
        mapView = MLNMapView(frame: view.bounds)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.delegate = self
        view.addSubview(mapView)
    }

    func mapView(_: MLNMapView, didFinishLoading style: MLNStyle) {
        guard let fileURL = Bundle.main.url(forResource: "world", withExtension: "pmtiles") else { return }
        let url = URL(string: "pmtiles://\(fileURL.absoluteString)")!
        let source = MLNVectorTileSource(identifier: "world", configurationURL: url)
        style.addSource(source)
        let layer = MLNFillStyleLayer(identifier: "countries", source: source)
        layer.sourceLayerIdentifier = "countries"
        layer.fillColor = NSExpression(forConstantValue: UIColor(red: 0.67, green: 0.83, blue: 0.87, alpha: 1))
        style.addLayer(layer)
    }
}
```
