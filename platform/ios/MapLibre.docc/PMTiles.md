# PMTiles

Working with PMTiles

Starting MapLibre iOS 6.10.0, using [PMTiles](https://docs.protomaps.com/pmtiles/) as a data source is supported. You can prefix your vector tile source with `pmtiles://` to load a PMTiles file. The rest of the URL continue with be `https://` to load a remote PMTiles file, `asset://` to load an asset or `file://` to load a local PMTiles file.

Oliver Wipfli has made a style available that combines a [Protomaps]() basemap togehter with Foursquare's POI dataset. It is available in the [wipfli/foursquare-os-places-pmtiles](https://github.com/wipfli/foursquare-os-places-pmtiles) repository on GitHub. The style to use is

```
https://raw.githubusercontent.com/wipfli/foursquare-os-places-pmtiles/refs/heads/main/style.json
```

The neat thing about this style is that it only uses PMTiles vector sources. PMTiles can be hosted with a relatively simple file server (or file hosting service) instead of a more complex specialized tile server.

![](pmtiles-demo.png)
