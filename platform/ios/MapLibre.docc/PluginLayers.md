# Create a Plugin Layer

Define and add custom plugin layers at runtime

Implement a ``MLNPluginLayer`` and add it to a ``MLNMapView`` with ``MLNMapView/addPluginLayerType:``. It is also possible to register a plugin layer at initialization using ``MLNMapOptions`` with ``MLNMapOptions/pluginLayers``.

Implement ``MLNPluginLayer/layerCapabilities`` to describe the structure of the style layer (see example below).

## Example 1: Triangles

This `PluginLayerExampleMetalRendering` can be found in the Objective-C based development app in the MapLibre Native repository.

Here we see the implementation of ``MLNPluginLayer/layerCapabilities``:

```objc
+(MLNPluginLayerCapabilities *)layerCapabilities {

    MLNPluginLayerCapabilities *tempResult = [[MLNPluginLayerCapabilities alloc] init];
    tempResult.layerID = @"plugin-layer-metal-rendering";
    tempResult.requiresPass3D = YES;

    // Define the paint properties that this layer implements and
    // what types they are
    tempResult.layerProperties = @[
        // The scale property
        [MLNPluginLayerProperty propertyWithName:@"scale"
                                    propertyType:MLNPluginLayerPropertyTypeSingleFloat
                                    defaultValue:@(1.0)],

        // The fill color property
        [MLNPluginLayerProperty propertyWithName:@"fill-color"
                                    propertyType:MLNPluginLayerPropertyTypeColor
                                    defaultValue:[UIColor blueColor]]

    ];

    return tempResult;

}
```

Here we see the layers that can now be added to the style to render triangles.

```json
{
  "id": "metal-rendering-layer-1",
  "type": "plugin-layer-metal-rendering",
  "properties": {
    "color1": "#FFAADD",
    "offset-x": -300,
    "offset-y": -300
  },
  "paint": {
    "scale": 2.5
  }
},
{
  "id": "metal-rendering-layer-2",
  "type": "plugin-layer-metal-rendering",
  "properties": {
    "color1": "#FFAADD",
    "offset-x": -300
  },
  "paint": {
    "scale": [
      "interpolate",
      [
        "linear"
      ],
      [
        "zoom"
      ],
      5,
      0.5,
      15,
      3
    ]
  }
},
{
  "id": "metal-rendering-layer-3",
  "type": "plugin-layer-metal-rendering",
  "properties": {
    "color1": "#DDAAFF",
    "offset-x": 300
  },
  "paint": {
    "scale": [
      "interpolate",
      [
        "linear"
      ],
      [
        "zoom"
      ],
      5,
      3,
      15,
      0.5
    ],
    "fill-color": [
      "interpolate",
      [
        "linear"
      ],
      [
        "zoom"
      ],
      1,
      "#ff0000",
      22,
      "#00ff00"
    ]
  }
}
```
![](PluginLayer.png)

## Example 2: GLTF Plugin Layer

In [this repository](https://github.com/AtlasProgramming/maplibre-gltf-models-plugin) you can find an example of a plugin layer that allows adding GLTF models. The following JSON is added to the style.

```
{
  "id": "model-layer",
  "type": "hudhud::gltf-model-layer",
  "properties": {
      "model-source-resource":"models.json"
  },
  "paint": {
      "scale": 1.0
  }
}
```

@Video(
   source: "gltfplugin.mp4",
   poster: "gltfplugin.png",
   alt: "GLTF Plugin in action.") {
    GLTF Plugin showing 3D models of the Arc de Triomphe and the Eifel Tower.
}
