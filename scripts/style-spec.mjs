import referenceSpec from "../scripts/style-spec-reference/v8.json" with { type: "json" };

// https://github.com/maplibre/maplibre-native/issues/2368
referenceSpec['layout_symbol']['icon-padding']['type'] = 'number';
// @ts-ignore
referenceSpec['layout_symbol']['icon-padding']['default'] = 2;

// https://github.com/maplibre/maplibre-native/issues/2754
referenceSpec['layout_symbol']['icon-padding']['property-type'] = 'data constant';

/** @type {any} */
let modifiedReferenceSpec = referenceSpec;

// https://github.com/maplibre/maplibre-native/issues/250
delete modifiedReferenceSpec['layout_symbol']['text-rotation-alignment']["values"]['viewport-glyph']

// https://github.com/maplibre/maplibre-native/issues/2358
delete modifiedReferenceSpec['layout_symbol']['text-variable-anchor-offset'];

// https://github.com/maplibre/maplibre-native/issues/251
delete modifiedReferenceSpec['layout_symbol']['icon-overlap'];
delete modifiedReferenceSpec['layout_symbol']['text-overlap'];
modifiedReferenceSpec["layout_symbol"]["text-allow-overlap"]["requires"] = referenceSpec["layout_symbol"]["text-allow-overlap"]["requires"].filter(val => JSON.stringify(val) !== '{"!":"text-overlap"}');
modifiedReferenceSpec["layout_symbol"]["text-allow-overlap"]["requires"] = referenceSpec["layout_symbol"]["icon-allow-overlap"]["requires"].filter(val => JSON.stringify(val) !== '{"!":"icon-overlap"}');

export default modifiedReferenceSpec;