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

// Ground shadows cast by fill-extrusion geometry.
//
// These live here rather than in scripts/style-spec-reference/v8.json because
// `npm run copy-style-spec` overwrites that file wholesale from the npm
// @maplibre/maplibre-gl-style-spec package and would silently delete them.
//
// All are `data-constant`: they live in a single per-layer uniform buffer, so they add no
// vertex attributes and no shader permutations. `minimum`/`maximum` below are documentation
// only -- nothing in core clamps them, so the renderer must.
modifiedReferenceSpec["paint_fill-extrusion"]["fill-extrusion-shadow-color"] = {
    "type": "color",
    "default": "rgba(0,0,0,0.35)",
    "doc": "The color of the ground shadow cast by the extruded geometry. Has no effect while `fill-extrusion-shadow-opacity` is 0.",
    "transition": true,
    "expression": {
        "interpolated": true,
        "parameters": [
            "zoom"
        ]
    },
    "property-type": "data-constant"
};

modifiedReferenceSpec["paint_fill-extrusion"]["fill-extrusion-shadow-opacity"] = {
    "type": "number",
    "default": 0,
    "minimum": 0,
    "maximum": 1,
    "doc": "The opacity of the ground shadow cast by the extruded geometry. A value of 0 disables the shadow entirely. This is rendered on a per-layer, not per-feature, basis.",
    "transition": true,
    "expression": {
        "interpolated": true,
        "parameters": [
            "zoom"
        ]
    },
    "property-type": "data-constant"
};

modifiedReferenceSpec["paint_fill-extrusion"]["fill-extrusion-shadow-length"] = {
    "type": "number",
    "default": 0.32,
    "minimum": 0,
    "doc": "The length of the ground shadow, as a multiple of the extrusion height. This is the cotangent of the light source's elevation angle, so 1 corresponds to an elevation of 45 degrees and 0.32 to roughly 72 degrees. Larger values cast longer, shallower shadows.",
    "transition": true,
    "expression": {
        "interpolated": true,
        "parameters": [
            "zoom"
        ]
    },
    "property-type": "data-constant"
};

modifiedReferenceSpec["paint_fill-extrusion"]["fill-extrusion-shadow-azimuth"] = {
    "type": "number",
    "default": 225,
    "period": 360,
    "units": "degrees",
    "doc": "The compass direction the ground shadow is cast towards, measured clockwise from due north. The shadow is anchored to the map, so it keeps a fixed compass direction as the map rotates.",
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
