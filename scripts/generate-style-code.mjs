import { parseArgs } from "node:util";
import * as path from "node:path";
import spec from "./style-spec.mjs";
import colorParser from "csscolorparser";

import { camelize, camelizeWithLeadingLowercase, readAndCompile, writeIfModified } from "./style-code.mjs";

// Parse command line
const args = (() => {
  const { values } = parseArgs({
    args: process.argv.slice(2),
    options: {
      out: {
        type: 'string',
        short: 'o'
      }
    },
    allowPositionals: false
  });
  return values;
})();

function parseCSSColor(/** @type {string} **/ str) {
  const color = colorParser.parseCSSColor(str);
  if (!color) throw new Error("Could not parse CSS color");
  return [
      color[0] / 255 * color[3], color[1] / 255 * color[3], color[2] / 255 * color[3], color[3]
  ];
}

/**
 * @param {any} property
 */
function isLightProperty(property) {
  return property['light-property'] === true;
};

/**
 * @param {any} property
 */
function isOverridable(property) {
    return ['text-color'].includes(property.name);
};

/**
 * @param {any} property
 * @returns {string}
 */
function expressionType(property) {
    switch (property.type) {
        case 'boolean':
            return 'BooleanType';
        case 'number':
        case 'enum':
            return 'NumberType';
        case 'image':
            return 'ImageType';
        case 'string':
            return 'StringType';
        case 'color':
            return `ColorType`;
        case 'padding':
            return `PaddingType`;
        case 'variableAnchorOffsetCollection':
            return `VariableAnchorOffsetCollectionType`;
        case 'formatted':
            return `FormattedType`;
        case 'array':
            return `Array<${expressionType({type: property.value})}>`;
        default: throw new Error(`unknown type for ${property.name}`)
    }
};

/**
 *
 * @param {any} property
 * @returns {string}
 */
function evaluatedType(property) {
  if (/text-variable-anchor-offset/.test(property.name)) {
    return 'VariableAnchorOffsetCollection';
  }
  if (/-translate-anchor$/.test(property.name)) {
    return 'TranslateAnchorType';
  }
  if (/-(rotation|pitch|illumination)-alignment$/.test(property.name)) {
    return 'AlignmentType';
  }
  if (/^(text|icon)-anchor$/.test(property.name)) {
    return 'SymbolAnchorType';
  }
  if (/position/.test(property.name)) {
    return 'Position';
  }
  switch (property.type) {
  case 'boolean':
    return 'bool';
  case 'number':
    // TODO: Check if 'Rotation' should be used for other properties,
    // such as icon-rotate
    if (/bearing$/.test(property.name) &&
        property.period == 360 &&
        property.units =='degrees') {
      return 'Rotation';
    }
    return /location$/.test(property.name) ? 'double' : 'float';
  case 'resolvedImage':
      return 'expression::Image';
  case 'formatted':
    return 'expression::Formatted';
  case 'string':
    return 'std::string';
  case 'enum':
    return (isLightProperty(property) ? 'Light' : '') + `${camelize(property.name)}Type`;
  case 'color':
    return `Color`;
  case 'padding':
    return `Padding`;
  case 'array':
    if (property.length) {
      return `std::array<${evaluatedType({type: property.value, name: property.name})}, ${property.length}>`;
    } else {
      return `std::vector<${evaluatedType({type: property.value, name: property.name})}>`;
    }
  default: throw new Error(`unknown type for ${property.name}`)
  }
};

/**
 * @param {any} property
 * @param {any} type
 */
function attributeUniformType(property, type) {
    /** @type {Record<string, string[]>} **/
    const attributeNameExceptions = {
      'text-opacity': ['opacity'],
      'icon-opacity': ['opacity'],
      'text-color': ['fill_color'],
      'icon-color': ['fill_color'],
      'text-halo-color': ['halo_color'],
      'icon-halo-color': ['halo_color'],
      'text-halo-blur': ['halo_blur'],
      'icon-halo-blur': ['halo_blur'],
      'text-halo-width': ['halo_width'],
      'icon-halo-width': ['halo_width'],
      'line-gap-width': ['gapwidth'],
      'line-pattern': ['pattern_to', 'pattern_from'],
      'line-floor-width': ['floorwidth'],
      'fill-pattern': ['pattern_to', 'pattern_from'],
      'fill-extrusion-pattern': ['pattern_to', 'pattern_from']
    }
    /** @type {string[]} **/
    const names = attributeNameExceptions[property.name] ||
       [ property.name.replace(type + '-', '').replace(/-/g, '_') ];

    return names.map(name => {
      return `attributes::${name}, uniforms::${name}`
    }).join(', ');
}

/**
 * @param {any} property
 */
function layoutPropertyType(property) {
  switch (property['property-type']) {
    case 'data-driven':
    case 'cross-faded-data-driven':
      return `DataDrivenLayoutProperty<${evaluatedType(property)}>`;
    default:
      return `LayoutProperty<${evaluatedType(property)}>`;
  }
};

/**
 *
 * @param {any} property
 * @param {any} type
 * @returns
 */
function paintPropertyType(property, type) {
  switch (property['property-type']) {
    case 'data-driven':
      if (isOverridable(property))
          return `DataDrivenPaintProperty<${evaluatedType(property)}, ${attributeUniformType(property, type)}, true>`;
      return `DataDrivenPaintProperty<${evaluatedType(property)}, ${attributeUniformType(property, type)}>`;
    case 'cross-faded-data-driven':
      return `CrossFadedDataDrivenPaintProperty<${evaluatedType(property)}, ${attributeUniformType(property, type)}>`;
    case 'cross-faded':
      return `CrossFadedPaintProperty<${evaluatedType(property)}>`;
    default:
      return `PaintProperty<${evaluatedType(property)}>`;
  }
};

/**
 * @param {any} property
 */
function propertyValueType(property) {
  switch (property['property-type']) {
    case 'color-ramp':
      return `ColorRampPropertyValue`;
    default:
      return `PropertyValue<${evaluatedType(property)}>`;
  }
};

/**
 * @param {any} property
 * @param {number} num
 * @returns {string}
 */
function formatNumber(property, num = 0) {
  if (evaluatedType(property) === "float") {
    const str = num.toString();
    return str + (str.includes(".") ? "" : ".") + "f";
  }
  return num.toString();
}

/**
 * @param {any} property
 */
function defaultValue(property) {
  // https://github.com/mapbox/mapbox-gl-native/issues/5258
  if (property.name === 'line-round-limit') {
    return 1;
  }

  if (property.name === 'fill-outline-color') {
    return '{}';
  }

  if (property['property-type'] === 'color-ramp') {
      return '{}';
  }

  switch (property.type) {
  case 'number':
    return formatNumber(property, property.default);
  case 'formatted':
  case 'string':
  case 'resolvedImage':
  case 'variableAnchorOffsetCollection':
    return property.default ? `{${JSON.stringify(property.default)}}` : '{}';
  case 'enum':
    if (property.default === undefined) {
      return `${evaluatedType(property)}::Undefined`;
    } else {
      return `${evaluatedType(property)}::${camelize(property.default)}`;
    }
  case 'color':
    const color = parseCSSColor(property.default).join(', ');
    switch (color) {
    case '0, 0, 0, 0':
      return '{}';
    case '0, 0, 0, 1':
      return 'Color::black()';
    case '1, 1, 1, 1':
      return 'Color::white()';
    default:
      return `{ ${color} }`;
    }
  case 'array':
  case 'padding':
    const defaults = (property.default || []).map((/** @type {any} **/ e) => defaultValue({ type: property.value, default: e }));
    if (property.length) {
      return `{{${defaults.join(', ')}}}`;
    } else {
      return `{${defaults.join(', ')}}`;
    }
  default:
    return property.default;
  }
};

console.log("Generating style code...");
const root = path.join(import.meta.dirname, "..")
const outLocation = args.out ? args.out : root;

const layerHpp = readAndCompile(`include/mbgl/style/layers/layer.hpp.ejs`, root);
const layerCpp = readAndCompile(`src/mbgl/style/layers/layer.cpp.ejs`, root);
const propertiesHpp = readAndCompile(`src/mbgl/style/layers/layer_properties.hpp.ejs`, root);
const propertiesCpp = readAndCompile(`src/mbgl/style/layers/layer_properties.cpp.ejs`, root);

const collator = new Intl.Collator("en-US");

// Add this mock property that our SDF line shader needs so that it gets added to the list of
// "data driven" properties.
spec.paint_line['line-floor-width'] = {
  "type": "number",
  "default": 1,
  "property-type": "data-driven"
};

const layers = Object.keys(spec.layer.type.values).map((type) => {

  /** @type {any[]} */
  const layoutProperties = Object.keys(spec[`layout_${type}`]).reduce((/** @type {any} **/ memo, name) => {
    if (name !== 'visibility') {
      spec[`layout_${type}`][name].name = name;
      memo.push(spec[`layout_${type}`][name]);
    }
    return memo;
  }, []);

  // JSON doesn't have a defined order. We're going to sort them alphabetically
  // to get a deterministic order.
  layoutProperties.sort((a, b) => collator.compare(a.name, b.name));

  /** @type {any[]} */
  const paintProperties = Object.keys(spec[`paint_${type}`]).reduce((/** @type {any} **/ memo, name) => {
    spec[`paint_${type}`][name].name = name;
    memo.push(spec[`paint_${type}`][name]);
    return memo;
  }, []);

  // JSON doesn't have a defined order. We're going to sort them alphabetically
  // to get a deterministic order.
  paintProperties.sort((a, b) => collator.compare(a.name, b.name));

  return {
    type: type,
    layoutProperties: layoutProperties,
    paintProperties: paintProperties,
    doc: spec.layer.type.values[type].doc,
    layoutPropertiesByName: spec[`layout_${type}`],
    paintPropertiesByName: spec[`paint_${type}`],
  };
});

function setupGlobalEjsHelpers() {
  const funcs = {
    layoutPropertyType,
    evaluatedType,
    isOverridable,
    expressionType,
    defaultValue,
    paintPropertyType,
    propertyValueType,
    camelize,
    camelizeWithLeadingLowercase
  };
  for (const [funcName, func] of Object.entries(funcs)) {
    // @ts-ignore
    global[funcName] = func;
  }
}

setupGlobalEjsHelpers();

for (let layer of layers) {
  const layerFileName = layer.type.replace('-', '_');

  writeIfModified(`src/mbgl/style/layers/${layerFileName}_layer_properties.hpp`, propertiesHpp(layer), outLocation);
  writeIfModified(`src/mbgl/style/layers/${layerFileName}_layer_properties.cpp`, propertiesCpp(layer), outLocation);

  // Remove our fake property for the external interace.
  if (layer.type === 'line') {
    layer.paintProperties = layer.paintProperties.filter(property => property.name !== 'line-floor-width');
  }

  writeIfModified(`include/mbgl/style/layers/${layerFileName}_layer.hpp`, layerHpp(layer), outLocation);
  writeIfModified(`src/mbgl/style/layers/${layerFileName}_layer.cpp`, layerCpp(layer), outLocation);
}

// Light
/** @type {any[]} **/
const lightProperties = Object.keys(spec[`light`]).reduce((/** @type {any} **/ memo, name) => {
  var property = spec[`light`][name];
  property.name = name;
  property['light-property'] = true;
  memo.push(property);
  return memo;
}, []);

// JSON doesn't have a defined order. We're going to sort them alphabetically
// to get a deterministic order.
lightProperties.sort((a, b) => collator.compare(a.name, b.name));

const lightHpp = readAndCompile(`include/mbgl/style/light.hpp.ejs`, root);
const lightCpp = readAndCompile(`src/mbgl/style/light.cpp.ejs`, root);
writeIfModified(`include/mbgl/style/light.hpp`, lightHpp({properties: lightProperties}), outLocation);
writeIfModified(`src/mbgl/style/light.cpp`, lightCpp({properties: lightProperties}), outLocation);
