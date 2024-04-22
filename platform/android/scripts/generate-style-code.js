#!/usr/bin/env node
'use strict';

const fs = require('fs');
const ejs = require('ejs');
const spec = require('../../../scripts/style-spec');
const _ = require('lodash');

// FIXME: https://github.com/mapbox/mapbox-gl-native/issues/15008
//delete spec.layout_circle["circle-sort-key"]
//delete spec.layout_line["line-sort-key"]
//delete spec.layout_fill["fill-sort-key"]

require('../../../scripts/style-code');

// Specification parsing //
const lightProperties = Object.keys(spec[`light`]).reduce((memo, name) => {
  var property = spec[`light`][name];
  property.name = name;
  property['light-property'] = true;
  property.doc = property.doc.replace(/°/g,'&#xB0;');
  memo.push(property);
  return memo;
}, []);

// Collect layer types from spec
var layers = Object.keys(spec.layer.type.values).map((type) => {
  const layoutProperties = Object.keys(spec[`layout_${type}`]).reduce((memo, name) => {
    if (name !== 'visibility' && type !== 'location-indicator') {
      spec[`layout_${type}`][name].name = name;
      memo.push(spec[`layout_${type}`][name]);
    }
    return memo;
  }, []);

  const paintProperties = Object.keys(spec[`paint_${type}`]).reduce((memo, name) => {
    if (type !== 'location-indicator') {
      spec[`paint_${type}`][name].name = name;
      memo.push(spec[`paint_${type}`][name]);
    }
    return memo;
  }, []);

  const locationLayoutProperties = Object.keys(spec[`layout_${type}`]).reduce((memo, name) => {
    if (name !== 'visibility' && type === 'location-indicator') {
      spec[`layout_${type}`][name].name = name;
      memo.push(spec[`layout_${type}`][name]);
    }
    return memo;
  }, []);

  const locationPaintProperties = Object.keys(spec[`paint_${type}`]).reduce((memo, name) => {
    if (type === 'location-indicator') {
      spec[`paint_${type}`][name].name = name;
      memo.push(spec[`paint_${type}`][name]);
    }
    return memo;
  }, []);

  return {
    type: type,
    doc: spec.layer.type.values[type].doc,
    layoutProperties: layoutProperties,
    paintProperties: paintProperties,
    locationLayoutProperties: locationLayoutProperties,
    locationPaintProperties: locationPaintProperties,
    properties: layoutProperties.concat(paintProperties).concat(locationLayoutProperties).concat(locationPaintProperties)
  };
});

// Process all layer properties
const uniqueArrayEnum = (prop, enums) => {
  if (prop.value !== 'enum') return false;
  const enumsEqual = (val1, val2) => val1.length === val1.length && val1.every((val, i) => val === val2[i]);
  return enums.filter(e => enumsEqual(Object.keys(prop.values).sort(), Object.keys(e.values).sort())).length == 0;
};

const layoutProperties = _(layers).map('layoutProperties').flatten().value();
const paintProperties = _(layers).map('paintProperties').flatten().value();
const locationLayoutProperties = _(layers).map('locationLayoutProperties').flatten().value();
const locationPaintProperties = _(layers).map('locationPaintProperties').flatten().value();
const allProperties = _(layoutProperties).union(paintProperties).union(lightProperties).value();
let allEnumProperties = _(allProperties).filter({'type': 'enum'}).value();
const uniqueArrayEnumProperties = _(allProperties).filter({'type': 'array'}).filter(prop => uniqueArrayEnum(prop, allEnumProperties)).value();
const enumProperties = _(allEnumProperties).union(uniqueArrayEnumProperties).value();


global.propertyType = function propertyType(property) {
  switch (property.type) {
      case 'boolean':
        return 'Boolean';
      case 'number':
        if (/bearing$/.test(property.name) &&
            property.period == 360 &&
            property.units =='degrees') {
          return 'Double';
        }
        return /location$/.test(property.name) ? 'Double' : 'Float';
      case 'formatted':
        return 'Formatted';
      case 'string':
      case 'resolvedImage':
        return 'String';
      case 'enum':
        return 'String';
      case 'color':
        return 'String';
      case 'array':
        return `${propertyType({type:property.value, name: property.name})}[]`;
      default:
        throw new Error(`unknown type for ${property.name}`);
  }
}

global.propertyJavaType = function propertyType(property) {
   switch (property.type) {
       case 'boolean':
         return 'boolean';
       case 'number':
         return 'float';
       case 'formatted':
         return 'Formatted';
       case 'string':
       case 'resolvedImage':
         return 'String';
       case 'enum':
         return 'String';
       case 'color':
         return 'String';
       case 'array':
         return `${propertyJavaType({type:property.value})}[]`;
       default:
         throw new Error(`unknown type for ${property.name}`);
   }
 }

global.propertyJNIType = function propertyJNIType(property) {
  switch (property.type) {
      case 'boolean':
        return 'jboolean';
      case 'number':
        return 'jfloat';
      case 'String':
        return 'String';
      case 'enum':
        return 'String';
      case 'color':
        return 'String';
      case 'array':
        return `jarray<${propertyType({type:property.value})}[]>`;
      default:
        return 'jobject*';
  }
}

global.propertyNativeType = function (property) {
  if (/-translate-anchor$/.test(property.name)) {
    return 'TranslateAnchorType';
  }
  if (/-(rotation|pitch|illumination)-alignment$/.test(property.name)) {
    return 'AlignmentType';
  }
  if (/^(text|icon)-anchor$/.test(property.name)) {
    return 'SymbolAnchorType';
  }
  switch (property.type) {
  case 'boolean':
    return 'bool';
  case 'number':
    return 'float';
  case 'formatted':
  case 'string':
  case 'resolvedImage': // TODO: replace once we implement image expressions
    return 'std::string';
  case 'enum':
    if(property['light-property']){
       return `Light${camelize(property.name)}Type`;
    }
    return `${camelize(property.name)}Type`;
  case 'color':
    return `Color`;
  case 'array':
    if (property.length) {
      return `std::array<${propertyType({type: property.value})}, ${property.length}>`;
    } else {
      return `std::vector<${propertyType({type: property.value})}>`;
    }
  default: throw new Error(`unknown type for ${property.name}`)
  }
}

global.propertyTypeAnnotation = function propertyTypeAnnotation(property) {
  switch (property.type) {
      case 'enum':
        return `@Property.${snakeCaseUpper(property.name)}`;
      default:
        return "";
  }
};

global.defaultExpressionJava = function(property) {
    switch (property.type) {
      case 'boolean':
        return 'boolean';
      case 'number':
        return 'number';
      case 'formatted':
        return 'format';
      case 'resolvedImage':
        return "image";
      case 'string':
      case 'image':
        return "string";
      case 'enum':
        return "string";
      case 'color':
        return 'toColor';
      case 'array':
        return "array";
      default: return "string";
      }
}

global.defaultValueJava = function(property) {
    if(property.name.endsWith("-pattern")) {
        return '"pedestrian-polygon"';
    }
    if(property.name.endsWith("-font")) {
        return 'new String[]{"Open Sans Regular", "Arial Unicode MS Regular"}';
    }
     switch (property.type) {
      case 'boolean':
        return 'true';
      case 'number':
        if (/bearing$/.test(property.name) &&
            property.period == 360 &&
            property.units =='degrees') {
          return '0.3';
        }
        return '0.3f';
      case 'formatted':
        return 'new Formatted(new FormattedSection("default"))'
      case 'string':
      case 'resolvedImage':
        return '"' + property['default'] + '"';
      case 'enum':
        return snakeCaseUpper(property.name) + "_" + snakeCaseUpper(Object.keys(property.values)[0]);
      case 'color':
        return '"rgba(255,128,0,0.7)"';
      case 'array':
             switch (property.value) {
              case 'string':
              case 'enum':
                if (property['default'] !== undefined) {
                    return '[' + property['default'] + ']';
                } else {
                    return 'new String[0]';
                }
              case 'number':
                var isDouble = /location$/.test(property.name)
                console.log(isDouble)
                var result = 'new ' + (isDouble ? 'Double' : 'Float') + '[] {';
                for (var i = 0; i < property.length; i++) {
                    result += isDouble ? '0.0' : '0f';
                    if (i +1 != property.length) {
                        result += ', ';
                    }
                }
                return result + '}';
             }
      default: throw new Error(`unknown type for ${property.name}`)
      }
}

/**
 * Produces documentation for property factory methods
 */
global.propertyFactoryMethodDoc = function (property) {
    var replaceIfPixels = function (doc) {
      return doc.replace('pixels', 'density-independent pixels')
    }
    let doc = replaceIfPixels(property.doc);
    // Match other items in back ticks
    doc = doc.replace(/`(.+?)`/g, function (m, symbol, offset, str) {
        if (str.substr(offset - 4, 3) !== 'CSS' && symbol[0].toUpperCase() != symbol[0] && _(enumProperties).filter({'name': symbol}).value().length > 0) {
            // Property 'enums'
            symbol = snakeCaseUpper(symbol);
            return '{@link Property.' + symbol + '}';
        } else if( _(allProperties).filter({'name': symbol}).value().length > 0) {
            // Other properties
            return '{@link PropertyFactory#' + camelizeWithLeadingLowercase(symbol) + '}';
        } else {
            // Left overs
            return '`' + symbol + '`';
        }
    });
    return doc;
};

/**
 * Produces documentation for property value constants
 */
global.propertyValueDoc = function (property, value) {

    // Match references to other property names & values.
    // Requires the format 'When `foo` is set to `bar`,'.
    let doc = property.values[value].doc.replace(/When `(.+?)` is set to `(.+?)`(?: or `([^`]+?)`)?,/g, function (m, peerPropertyName, propertyValue, secondPropertyValue, offset, str) {
        let otherProperty = snakeCaseUpper(peerPropertyName);
        let otherValue = snakeCaseUpper(peerPropertyName) + '_' + snakeCaseUpper(propertyValue);
        const firstPropertyValue = 'When {@link ' + `${otherProperty}` + '} is set to {@link Property#' + `${otherValue}` + '}';
        if (secondPropertyValue) {
            return firstPropertyValue + ` or {@link Property#${snakeCaseUpper(peerPropertyName) + '_' + snakeCaseUpper(secondPropertyValue)}},`;
        } else {
            return firstPropertyValue + ',';
        }
    });

    // Match references to our own property values.
    // Requires the format 'is equivalent to `bar`'.
    doc = doc.replace(/is equivalent to `(.+?)`/g, function(m, propertyValue, offset, str) {
        propertyValue = snakeCaseUpper(property.name) + '_' + snakeCaseUpper(propertyValue);
        return 'is equivalent to {@link Property#' + propertyValue + '}';
    });

    // Match other items in back ticks
    doc = doc.replace(/`(.+?)`/g, function (m, symbol, offset, str) {
        if ('values' in property && Object.keys(property.values).indexOf(symbol) !== -1) {
            // Property values
            propertyValue = snakeCaseUpper(property.name) + '_' + snakeCaseUpper(symbol);
            console.log("Transforming", symbol, propertyValue);
            return '{@link Property#' + `${propertyValue}` + '}';
        } else if (str.substr(offset - 4, 3) !== 'CSS' && symbol[0].toUpperCase() != symbol[0]) {
            // Property 'enums'
            if (symbol === 'symbol-sort-key') {
               return 'symbol sort key';
            }

            symbol = snakeCaseUpper(symbol);
            return '{@link ' + symbol + '}';
        } else {
            // Left overs
            return symbol
        }
    });
    return doc;
};

global.isLightProperty = function (property) {
  return property['light-property'] === true;
};

global.propertyValueType = function (property) {
  switch (property['property-type']) {
    default:
      return `PropertyValue<${evaluatedType(property)}>`;
  }
};

global.evaluatedType = function (property) {
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
    return 'float';
  case 'formatted':
  case 'string':
  case 'image':
    return 'std::string';
  case 'enum':
    return (isLightProperty(property) ? 'Light' : '') + `${camelize(property.name)}Type`;
  case 'color':
    return `Color`;
  case 'array':
    if (property.length) {
      return `std::array<${evaluatedType({type: property.value})}, ${property.length}>`;
    } else {
      return `std::vector<${evaluatedType({type: property.value})}>`;
    }
  default: throw new Error(`unknown type for ${property.name}`)
  }
};

global.supportsZoomFunction = function (property) {
  return property.expression && property.expression.parameters.indexOf('zoom') > -1;
};

global.supportsPropertyFunction = function (property) {
  return property['property-type'] === 'data-driven' || property['property-type'] === 'cross-faded-data-driven';
};

// Template processing //

// Java + JNI Light (Peer model)
const lightJava = ejs.compile(fs.readFileSync('MapLibreAndroid/src/main/java/org/maplibre/android/style/light/light.java.ejs', 'utf8'), {strict: true});
const lightJavaUnitTests = ejs.compile(fs.readFileSync('MapLibreAndroidTestApp/src/androidTest/java/org/maplibre/android/testapp/style/light.junit.ejs', 'utf8'), {strict: true});
writeIfModified(`MapLibreAndroid/src/main/java/org/maplibre/android/style/light/Light.java`, lightJava({properties: lightProperties}));
writeIfModified(`MapLibreAndroidTestApp/src/androidTest/java/org/maplibre/android/testapp/style/LightTest.java`, lightJavaUnitTests({properties: lightProperties}));

// Java
const layerJava = ejs.compile(fs.readFileSync('MapLibreAndroid/src/main/java/org/maplibre/android/style/layers/layer.java.ejs', 'utf8'), {strict: true});
const layerJavaUnitTests = ejs.compile(fs.readFileSync('MapLibreAndroidTestApp/src/androidTest/java/org/maplibre/android/testapp/style/layer.junit.ejs', 'utf8'), {strict: true});

for (const layer of layers) {
  var srcDir = 'MapLibreAndroid/src/main/java/org/maplibre/android/style/layers/'
  var testDir = 'MapLibreAndroidTestApp/src/androidTest/java/org/maplibre/android/testapp/style/'
  if (layer.type === 'location-indicator') {
    srcDir = 'MapLibreAndroid/src/main/java/org/maplibre/android/location/'
    testDir = 'MapLibreAndroidTestApp/src/androidTest/java/org/maplibre/android/location/'
  }

  writeIfModified(srcDir + `${camelize(layer.type)}Layer.java`, layerJava(layer));
  writeIfModified(testDir + `${camelize(layer.type)}LayerTest.java`, layerJavaUnitTests(layer));
}

// Jni
const layerHpp = ejs.compile(fs.readFileSync('MapLibreAndroid/src/cpp/style/layers/layer.hpp.ejs', 'utf8'), {strict: true});
const layerCpp = ejs.compile(fs.readFileSync('MapLibreAndroid/src/cpp/style/layers/layer.cpp.ejs', 'utf8'), {strict: true});

for (const layer of layers) {
  const layerFileName = layer.type.replace('-', '_');

  writeIfModified(`MapLibreAndroid/src/cpp/style/layers/${layerFileName}_layer.hpp`, layerHpp(layer));
  writeIfModified(`MapLibreAndroid/src/cpp/style/layers/${layerFileName}_layer.cpp`, layerCpp(layer));
}

// Java PropertyFactory
const propertyFactoryTemplate = ejs.compile(fs.readFileSync('MapLibreAndroid/src/main/java/org/maplibre/android/style/layers/property_factory.java.ejs', 'utf8'), {strict: true});

var propertyFactorySrcDir = 'MapLibreAndroid/src/main/java/org/maplibre/android/style/layers/PropertyFactory.java'
writeIfModified(
    propertyFactorySrcDir,
    propertyFactoryTemplate({layoutProperties: layoutProperties, paintProperties: paintProperties, locationIndicator: false})
);

var locationPropertyFactorySrcDir = 'MapLibreAndroid/src/main/java/org/maplibre/android/location/LocationPropertyFactory.java'
writeIfModified(
    locationPropertyFactorySrcDir,
    propertyFactoryTemplate({layoutProperties: locationLayoutProperties, paintProperties: locationPaintProperties, locationIndicator: true})
);

// Java Property
const enumPropertyJavaTemplate = ejs.compile(fs.readFileSync('MapLibreAndroid/src/main/java/org/maplibre/android/style/layers/property.java.ejs', 'utf8'), {strict: true});
writeIfModified(
    `MapLibreAndroid/src/main/java/org/maplibre/android/style/layers/Property.java`,
    enumPropertyJavaTemplate({properties: enumProperties})
);
