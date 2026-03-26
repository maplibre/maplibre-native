# ``MapLibre``

@Metadata {
    @Available(iOS, introduced: "12.0")
}

@Options(scope: global) {
  @AutomaticArticleSubheading(disabled)
}

Powerful, free and open-source mapping toolkit with full control over data sources and styling.

## Overview

[MapLibre Native](https://github.com/maplibre/maplibre-native) is a map rendering toolkit with support for iOS. It can be used as an alternative to MapKit. You have full control over the data sources used for rendering the map, as well as the styling. You can even participate in the development as MapLibre Native is free and open-source project.
> Note: For information on creating and modifying map styles, see the [MapLibre Style Spec documentation](https://maplibre.org/maplibre-style-spec/).

## License

MapLibre iOS is distributed under the BSD 2-Clause License. Refer to the [full list of licenses](https://github.com/maplibre/maplibre-native/blob/main/platform/ios/LICENSE.md).

MapLibre iOS is based in part of the work of the [FreeType](https://freetype.org/) Team.

## Topics

### Essentials

- <doc:GettingStarted>
- <doc:AddMarkerSymbolExample>

### Styling and Dynamic Data

- <doc:AnimatedLineExample>
- <doc:WebAPIDataExample>
- <doc:LineStyleLayerExample>
- <doc:DDSCircleLayerExample>
- <doc:POIAlongRouteExample>
- <doc:GeoJSON>
- <doc:PMTiles>

### Map Interaction

- <doc:LineOnUserTap>
- <doc:BlockingGesturesExample>
- <doc:AnnotationViewExample>
- <doc:BuildingLightExample>

### Features

- <doc:LocationPrivacyExample>
- <doc:StaticSnapshotExample>

### Offline

- <doc:OfflinePackExample>
- <doc:ManageOfflineRegionsExample>

### Observability

- <doc:ObserverExample>
- <doc:ActionJournalExample>

### Advanced

- <doc:CustomStyleLayerExample>
- <doc:RenderingStatisticsHud>
- <doc:PluginLayers>

### Other Articles

- <doc:Customizing_Fonts>
- <doc:Info.plist_Keys>
- <doc:GestureRecognizers>
- <doc:MultipleImagesExample>
- <doc:Predicates_and_Expressions>
- <doc:Tile_URL_Templates>
- <doc:For_Style_Authors>

### Map

- ``MLNSettings``
- ``MLNMapCamera``
- ``MLNMapViewDelegate``
- ``MLNMapView``
- ``MLNUserTrackingMode``

### Style Layers

- ``MLNBackgroundStyleLayer``
- ``MLNCircleStyleLayer``
- ``MLNFillExtrusionStyleLayer``
- ``MLNFillStyleLayer``
- ``MLNForegroundStyleLayer``
- ``MLNHeatmapStyleLayer``
- ``MLNHillshadeStyleLayer``
- ``MLNLineStyleLayer``
- ``MLNRasterStyleLayer``
- ``MLNStyleLayer``
- ``MLNSymbolStyleLayer``
- ``MLNVectorStyleLayer``

### Sources

- ``MLNComputedShapeSource``
- ``MLNImageSource``
- ``MLNRasterDEMSource``
- ``MLNRasterTileSource``
- ``MLNShapeSource``
- ``MLNSource``
- ``MLNTileSource``

### Shapes

- ``MLNEmptyFeature``
- ``MLNMultiPoint``
- ``MLNMultiPolygon``
- ``MLNMultiPolygonFeature``
- ``MLNMultiPolyline``
- ``MLNMultiPolylineFeature``
- ``MLNMultiPolylineFeature``
- ``MLNPointAnnotation``
- ``MLNPointCollection``
- ``MLNPointCollectionFeature``
- ``MLNPolygon``
- ``MLNPolyline``
- ``MLNPolylineFeature``
- ``MLNShape``
- ``MLNShapeCollection``
- ``MLNShapeCollectionFeature``

### Snapshotter

- ``MLNMapSnapshot``
- ``MLNMapSnapshotOptions``
- ``MLNMapSnapshotter``

### Offline support

- ``MLNOfflinePack``
- ``MLNOfflineRegion``
- ``MLNOfflineStorage``
- ``MLNShapeOfflineRegion``
- ``MLNTilePyramidOfflineRegion``

### Annotations

- ``MLNAnnotationImage``
- ``MLNAnnotationView``
- ``MLNPointFeature``
- ``MLNPointFeatureCluster``
