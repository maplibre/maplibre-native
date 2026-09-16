import referenceSpec from './style-spec-reference/v8.json' with { type: "json" };

/** @type {any} */
let modifiedReferenceSpec = referenceSpec;

// https://github.com/maplibre/maplibre-native/issues/250
delete modifiedReferenceSpec['layout_symbol']['text-rotation-alignment']["values"]['viewport-glyph']

// https://github.com/maplibre/maplibre-native/issues/251
delete modifiedReferenceSpec['layout_symbol']['icon-overlap'];
delete modifiedReferenceSpec['layout_symbol']['text-overlap'];
modifiedReferenceSpec["layout_symbol"]["text-allow-overlap"]["requires"] = referenceSpec["layout_symbol"]["text-allow-overlap"]["requires"].filter(val => JSON.stringify(val) !== '{"!":"text-overlap"}');
modifiedReferenceSpec["layout_symbol"]["text-allow-overlap"]["requires"] = referenceSpec["layout_symbol"]["icon-allow-overlap"]["requires"].filter(val => JSON.stringify(val) !== '{"!":"icon-overlap"}');

// https://github.com/maplibre/maplibre-native/issues/1929
delete modifiedReferenceSpec['layout_symbol']['symbol-height-offset'];
delete modifiedReferenceSpec['layout_symbol']['symbol-height-anchor'];

// https://github.com/maplibre/maplibre-native/issues/4117
delete modifiedReferenceSpec['paint_raster']['resampling'];
delete modifiedReferenceSpec['paint_hillshade']['resampling'];
delete modifiedReferenceSpec['paint_color-relief']['resampling'];

// https://github.com/maplibre/maplibre-native/issues/4298
delete modifiedReferenceSpec['paint_fill']['fill-layer-opacity'];
delete modifiedReferenceSpec['paint_line']['line-layer-opacity'];

// https://github.com/maplibre/maplibre-native/issues/4194
for (const name of ['line-cap', 'line-miter-limit', 'line-round-limit']) {
    modifiedReferenceSpec['layout_line'][name]['property-type'] = 'data-constant';
    modifiedReferenceSpec['layout_line'][name].expression.parameters = ['zoom'];
}
modifiedReferenceSpec['paint_line']['line-dasharray']['property-type'] = 'cross-faded';
modifiedReferenceSpec['paint_line']['line-dasharray'].expression.parameters = ['zoom'];

// https://github.com/maplibre/maplibre-native/issues/4517
modifiedReferenceSpec['layout_symbol']['icon-rotation-alignment']['property-type'] = 'data-constant';
modifiedReferenceSpec['layout_symbol']['icon-rotation-alignment'].expression.parameters = ['zoom'];

modifiedReferenceSpec.layer.type.values["location-indicator"] = {};
modifiedReferenceSpec["layout_location-indicator"] = {
  "top-image": {
      "type": "resolvedImage",
      "property-type": "data-constant",
      "expression": {
          "interpolated": false,
          "parameters": [
              "zoom"
          ]
      },
      "doc": "Name of image in sprite to use as the top of the location indicator."
  },
  "bearing-image": {
      "type": "resolvedImage",
      "property-type": "data-constant",
      "expression": {
          "interpolated": false,
          "parameters": [
              "zoom"
          ]
      },
      "doc": "Name of image in sprite to use as the middle of the location indicator."
  },
  "shadow-image": {
      "type": "resolvedImage",
      "property-type": "data-constant",
      "expression": {
          "interpolated": false,
          "parameters": [
              "zoom"
          ]
      },
      "doc": "Name of image in sprite to use as the background of the location indicator."
  }
};

modifiedReferenceSpec["paint_location-indicator"] = {
  "perspective-compensation": {
      "type": "number",
      "default": "0.85",
      "property-type": "data-constant",
      "expression": {
          "interpolated": true,
          "parameters": [
              "zoom"
          ]
      },
      "doc": "The amount of the perspective compensation, between 0 and 1. A value of 1 produces a location indicator of constant width across the screen. A value of 0 makes it scale naturally according to the viewing projection."
  },
  "image-tilt-displacement": {
      "type": "number",
      "property-type": "data-constant",
      "default": "0",
      "units": "pixels",
      "expression": {
          "interpolated": true,
          "parameters": [
              "zoom"
          ]
      },
      "doc": "The displacement off the center of the top image and the shadow image when the pitch of the map is greater than 0. This helps producing a three-dimensional appearance."
  },
  "bearing": {
      "type": "number",
      "default": 0,
      "period": 360,
      "units": "degrees",
      "property-type": "data-constant",
      "expression": {
          "interpolated": false,
          "parameters": [ ]
      },
      "transition": true,
      "doc": "The bearing of the location indicator."
  },
  "location": {
      "type": "array",
      "default": [
          0.0,
          0.0,
          0.0
      ],
      "length": 3,
      "value": "number",
      "property-type": "data-constant",
      "expression": {
          "interpolated": true,
          "parameters": []
      },
      "transition": true,
      "doc": "An array of [latitude, longitude, altitude] position of the location indicator."
  },
  "accuracy-radius": {
      "type": "number",
      "units": "meters",
      "default": 0,
      "property-type": "data-constant",
      "expression": {
          "interpolated": true,
          "parameters": [
              "zoom"
          ]
      },
      "transition": true,
      "doc": "The accuracy, in meters, of the position source used to retrieve the position of the location indicator."
  },
  "top-image-size": {
      "type": "number",
      "units": "factor of the original icon size",
      "property-type": "data-constant",
      "default": 1,
      "expression": {
          "interpolated": true,
          "parameters": [
              "zoom"
          ]
      },
      "transition": true,
      "doc": "The size of the top image, as a scale factor applied to the size of the specified image."
  },
  "bearing-image-size": {
      "type": "number",
      "units": "factor of the original icon size",
      "property-type": "data-constant",
      "default": 1,
      "expression": {
          "interpolated": true,
          "parameters": [
              "zoom"
          ]
      },
      "transition": true,
      "doc": "The size of the bearing image, as a scale factor applied to the size of the specified image."
  },
  "shadow-image-size": {
      "type": "number",
      "units": "factor of the original icon size",
      "property-type": "data-constant",
      "default": 1,
      "expression": {
          "interpolated": true,
          "parameters": [
              "zoom"
          ]
      },
      "transition": true,
      "doc": "The size of the shadow image, as a scale factor applied to the size of the specified image."
  },
  "accuracy-radius-color": {
      "type": "color",
      "property-type": "data-constant",
      "default": "#ffffff",
      "expression": {
          "interpolated": true,
          "parameters": [
              "zoom"
          ]
      },
      "transition": true,
      "doc": "The color for drawing the accuracy radius, as a circle. To adjust transparency, set the alpha component of the color accordingly."

  },
  "accuracy-radius-border-color": {
      "type": "color",
      "property-type": "data-constant",
      "default": "#ffffff",
      "expression": {
          "interpolated": true,
          "parameters": [
              "zoom"
          ]
      },
      "transition": true,
      "doc": "The color for drawing the accuracy radius border. To adjust transparency, set the alpha component of the color accordingly."
  }
};

// internal use
modifiedReferenceSpec["layout_symbol"]["symbol-screen-space"] = {
    "type": "boolean",
    "default": false,
    "property-type": "data-constant",
    "doc": "Internal use only"
};

export default modifiedReferenceSpec;
