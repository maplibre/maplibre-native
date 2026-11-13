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

// Ambient occlusion of fill-extrusions, see
// https://github.com/maplibre/maplibre-style-spec/issues/1796#issuecomment-6194201898. Takes ambient
// light away from the walls near the ground and darkens the ground around the footprints; extrusions
// standing on other extrusions are not occluded. All four are `data-constant` and live in per-layer
// uniform buffers.
// Not in the published spec yet.
modifiedReferenceSpec["paint_fill-extrusion"]["fill-extrusion-ambient-occlusion-wall-intensity"] = {
    "type": "number",
    "default": 0,
    "minimum": 0,
    "maximum": 1,
    "doc": "How much of the ambient light is taken away from the bottom of the extrusion's walls, where they meet the ground. At `1`, the bottom of the walls gets only the directional light. The ambient part of the lighting is `1` minus the light's `intensity`, so the effect is stronger with a weaker light and disappears at an intensity of `1`. A value of `0` disables the effect. Walls of parts that do not reach the ground, i.e. with a `fill-extrusion-base` above the radius, are not occluded.",
    "transition": true,
    "expression": {
        "interpolated": true,
        "parameters": [
            "zoom"
        ]
    },
    "property-type": "data-constant"
};

modifiedReferenceSpec["paint_fill-extrusion"]["fill-extrusion-ambient-occlusion-wall-radius"] = {
    "type": "number",
    "default": 3,
    "minimum": 0,
    "units": "meters",
    "doc": "Height in meters above the ground over which the ambient occlusion of the walls fades out.",
    "transition": true,
    "expression": {
        "interpolated": true,
        "parameters": [
            "zoom"
        ]
    },
    "property-type": "data-constant"
};

modifiedReferenceSpec["paint_fill-extrusion"]["fill-extrusion-ambient-occlusion-ground-intensity"] = {
    "type": "number",
    "default": 0,
    "minimum": 0,
    "maximum": 1,
    "doc": "How much the ground is darkened around the footprint of the extrusions. A value of `0` disables the effect. Only parts that stand on the ground darken it. Needs an opaque `background` layer at the bottom of the style, and has no effect without one. Not supported by the OpenGL renderer, which leaves the ground as is.",
    "transition": true,
    "expression": {
        "interpolated": true,
        "parameters": [
            "zoom"
        ]
    },
    "property-type": "data-constant"
};

modifiedReferenceSpec["paint_fill-extrusion"]["fill-extrusion-ambient-occlusion-ground-radius"] = {
    "type": "number",
    "default": 3,
    "minimum": 0,
    "units": "meters",
    "doc": "Distance in meters from the walls over which the darkening of the ground fades out.",
    "transition": true,
    "expression": {
        "interpolated": true,
        "parameters": [
            "zoom"
        ]
    },
    "property-type": "data-constant"
};

export default modifiedReferenceSpec;
