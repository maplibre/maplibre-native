# ``MLNCoordinateReferenceSystem``

## Drawing the Arctic in EPSG:3413

NASA GIBS serves imagery of the Arctic in EPSG:3413, a polar stereographic projection centered on the North Pole. This example defines that projection, registers it with ``MLNStyle/addProjection:error:``, and loads a style that names it in `projection.type`. The raster source asks GIBS for each tile by its `{bbox}`, the tile's corners in the projection's meters.

<!-- include-example(CoordinateReferenceSystemExample) -->

```swift
/// EPSG:3413, NSIDC's north polar stereographic projection of the WGS 84 ellipsoid, on the grid
/// NASA GIBS serves it in. Formulas from Snyder, Map Projections: A Working Manual (1987),
/// 21-34 and 21-38 to 21-41.
let arcticProjection: MLNCoordinateReferenceSystem = {
    let flattening = 1 / 298.257223563
    let e2 = flattening * (2 - flattening)
    let e = e2.squareRoot()
    let centralLongitude = -45.0
    let radians = Double.pi / 180

    func t(_ latitude: Double) -> Double {
        let sinLatitude = sin(latitude)
        return tan(.pi / 4 - latitude / 2) / pow((1 - e * sinLatitude) / (1 + e * sinLatitude), e / 2)
    }

    let trueScaleLatitude = 70 * radians
    let sinTrueScale = sin(trueScaleLatitude)
    let m = cos(trueScaleLatitude) / (1 - e2 * sinTrueScale * sinTrueScale).squareRoot()
    let rhoPerT = 6_378_137 * m / t(trueScaleLatitude)

    return MLNCoordinateReferenceSystem(
        name: "EPSG:3413",
        tileMatrixSet: MLNTileMatrixSet(
            origin: MLNCRSPosition(x: -4_194_304, y: 4_194_304),
            extentAtZoom0: 8_388_608
        ),
        forward: { coordinate in
            let rho = rhoPerT * t(coordinate.latitude * radians)
            let angle = (coordinate.longitude - centralLongitude) * radians
            return MLNCRSPosition(x: rho * sin(angle), y: -rho * cos(angle))
        },
        inverse: { position in
            let e4 = e2 * e2
            let e6 = e4 * e2
            let e8 = e6 * e2
            let chi = .pi / 2 - 2 * atan(hypot(position.x, position.y) / rhoPerT)
            var latitude = chi
            latitude += (e2 / 2 + 5 * e4 / 24 + e6 / 12 + 13 * e8 / 360) * sin(2 * chi)
            latitude += (7 * e4 / 48 + 29 * e6 / 240 + 811 * e8 / 11520) * sin(4 * chi)
            latitude += (7 * e6 / 120 + 81 * e8 / 1120) * sin(6 * chi)
            latitude += 4279 * e8 / 161_280 * sin(8 * chi)
            return CLLocationCoordinate2D(
                latitude: latitude / radians,
                longitude: centralLongitude + atan2(position.x, -position.y) / radians
            )
        }
    )
}()

class CoordinateReferenceSystemExample: UIViewController {
    override func viewDidLoad() {
        super.viewDidLoad()

        // Every map shares the registry, and a name is registered once, so a second visit's
        // error is expected. Register before loading a style that names the projection.
        try? MLNStyle.addProjection(arcticProjection)

        let styleJSON = """
        {
          "version": 8,
          "projection": {"type": "EPSG:3413"},
          "center": [-45, 90],
          "zoom": 1,
          "sources": {
            "blue-marble": {
              "type": "raster",
              "tiles": ["https://gibs.earthdata.nasa.gov/wms/epsg3413/best/wms.cgi?SERVICE=WMS&REQUEST=GetMap&VERSION=1.3.0&LAYERS=BlueMarble_ShadedRelief_Bathymetry&STYLES=&FORMAT=image/jpeg&CRS=EPSG:3413&BBOX={bbox}&WIDTH=512&HEIGHT=512"],
              "tileSize": 512
            }
          },
          "layers": [{"id": "blue-marble", "type": "raster", "source": "blue-marble"}]
        }
        """
        let mapView = MLNMapView(frame: view.bounds, styleJSON: styleJSON)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        view.addSubview(mapView)
    }
}
```
