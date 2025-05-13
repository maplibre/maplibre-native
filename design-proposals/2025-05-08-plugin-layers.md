# Plug-In Layer Architecture Design Proposal

## Motivation

This project is to add the ability to "register" additional layer types at runtime and have them integrated into the standard parameter/rendering pipeline.

## Proposed Change

For the initial implemention, the following functionality is proposed:

* At the platform level, be able to register a plug-in layer that is then parseable by the style parser
* The plug-in layer will be limited to simple rendering (via handing off the rendering context to the plug-in layer) and will not include the ability to define drawables/etc
* The "paint" properties will be parseable, support expressions and passed in frame by frame
* A custom set of "plugin-propeties" will also be available at the same level as the "paint" properties
* The plug-in layer will be notified about lifecycle events (creation, addition to the mapview, removal, destruction/etc) and be expected to manage it's own resources

Future features:
* Placeholder for ideas that could be implemented in the future

## Example IOS Utilization
An example implemention is shown in the PR: https://github.com/maplibre/maplibre-native/pull/3430
The platform/darwin/app/PluginLayerExampleMetalRendering.h/mm class shows how the layer manages it's own rendering and how properties from the style are passed to it.  In platform/ios/app/MBXViewController.mm there's a single line where the plug-in layer class is registered with the map view
```
    [self.mapView addPluginLayerType:[PluginLayerExampleMetalRendering class]];
```

The layer is then added to the style in the platform/darwin/app/PluginLayerTestStyleSimple.json file and an expression based scale property
is added

```
 {   "id": "metal-rendering-layer-1",
            "type": "plugin-layer-metal-rendering",
            "properties": {
                "color1":"#FFAADD",
                "offset-x": -300
            },
            "paint": {
                "scale": [
                    "interpolate",
                    ["linear"],
                    ["zoom"],
                    5,
                    0.5,
                    15,
                    3.0
                ]
            }
        },
```

That scale property is then evaluated by the internal implementation and passed back to the plug-in layer via the onUpdateLayerProperties virtual method where it's incorporated by the rendering.
```
-(void)onUpdateLayerProperties:(NSDictionary *)layerProperties {
    NSLog(@"Metal Layer Rendering Properties: %@", layerProperties);

    NSNumber *offsetX = [layerProperties objectForKey:@"offset-x"];
    if (offsetX) {
        _offsetX = [[layerProperties objectForKey:@"offset-x"] floatValue];
    }
    
    NSNumber *scale = [layerProperties objectForKey:@"scale"];
    if (scale) {
        if ([scale isKindOfClass:[NSNumber class]]) {
            _scale = [scale floatValue];
        }
    }

}
```

## API Modifications

The platform layer would provide a base class for implementing the plug-in layer.  The mapview would have a new method for registering the plug-in layer class.


## Migration Plan and Compatibility

All API changes are additive, so, no backwards compatibility issues should be present.

## Rejected Alternatives

N/A
