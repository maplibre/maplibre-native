# PMTiles

Working with PMTiles

Starting MapLibre iOS 6.10.0, using [PMTiles](https://docs.protomaps.com/pmtiles/) as a data source is supported. You can prefix your vector tile source with `pmtiles://` to load a PMTiles file. The rest of the URL can use `https://` to load a remote PMTiles file, `asset://` to load an asset, or `file://` to load a local PMTiles file.

> Note: PMTiles sources currently do not support caching or offline pack downloads.

## Loading a style that uses PMTiles sources

The simplest approach is to load a style JSON that already references PMTiles sources. Pass the style URL when initializing `MLNMapView`:

```swift
let styleURL = URL(string: "https://example.com/style.json")!
let mapView = MLNMapView(frame: view.bounds, styleURL: styleURL)
```

For a working example style that combines a Protomaps basemap with Foursquare's open POI dataset, see the [wipfli/foursquare-os-places-pmtiles](https://github.com/wipfli/foursquare-os-places-pmtiles) repository.

## Adding a PMTiles source programmatically

Use `MLNVectorTileSource` with a `pmtiles://` configuration URL to add a PMTiles archive as a vector source to an existing style:

```swift
func mapViewDidFinishLoadingMap(_ mapView: MLNMapView) {
    guard let style = mapView.style else { return }
    let sourceURL = URL(string: "pmtiles://https://example.com/tiles.pmtiles")!
    let source = MLNVectorTileSource(identifier: "my-pmtiles", configurationURL: sourceURL)
    style.addSource(source)
    // Add layers referencing "my-pmtiles"
}
```

PMTiles can be hosted on a simple static file server or CDN instead of a specialized tile server.

![Screenshot of PMTiles based style using Protomaps basemap with Foursquare POIs](pmtiles-demo.png)
