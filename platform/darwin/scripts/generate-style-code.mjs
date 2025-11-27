import { parseArgs } from "node:util";
import path from "node:path";
import _ from "lodash";
import colorParser from "csscolorparser";
import assert from "assert";

import { readAndCompile, writeIfModified, camelize, unhyphenate } from "../../../scripts/style-code.mjs";

import styleSpec from "../../../scripts/style-spec.mjs";

delete styleSpec.layer.type.values["location-indicator"];
delete styleSpec["layout_location-indicator"]
delete styleSpec["paint_location-indicator"];

import cocoaConventions from './style-spec-cocoa-conventions-v8.json' with { type: "json" };
import styleSpecOverrides from './style-spec-overrides-v8.json' with { type: "json" };

function setupGlobalEjsHelpers() {
    const funcs = {
      camelize,
      unhyphenate
    };
    for (const [funcName, func] of Object.entries(funcs)) {
      // @ts-ignore
      global[funcName] = func;
    }
  }

  setupGlobalEjsHelpers();

// Parse command line
const args = parseArgs({
    options: {
        out: {
            type: 'string',
            short: 'o',
            description: 'Directory root to write generated code.'
        }
    },
    allowPositionals: false
});

const prefix = 'MLN';
const suffix = 'StyleLayer';

let spec = _.merge(styleSpec, styleSpecOverrides);

class ConventionOverride {
    constructor(val) {
        if (typeof val === 'string') {
            this.name_ = val;
            this.enumName_ = null;
        } else if (val instanceof Object) {
            this.name_ = val.name;
            this.enumName_ = val.enumName;
        } else {
            assert(false);
        }
    }

    set name(name_) { this.name_ = name_; }
    get name() { return this.name_; }
    get enumName() { return this.enumName_ || this.name_; }
}

// Rename properties and keep `original` for use with setters and getters
_.forOwn(cocoaConventions, function (properties, kind) {
    _.forOwn(properties, function (newConvention, oldName) {
        let conventionOverride = new ConventionOverride(newConvention);
        let property = spec[kind][oldName];

        if (property) {
            if (conventionOverride.name.startsWith('is-')) {
                property.getter = conventionOverride.name;
                conventionOverride.name = conventionOverride.name.substr(3);
            }

            // Override enum name based on style-spec-cocoa-conventions-v8.json
            property.enumName = conventionOverride.enumName;

            if (conventionOverride.name !== oldName) {
                property.original = oldName;
                delete spec[kind][oldName];
                spec[kind][conventionOverride.name] = property;
            }

            // Update cross-references to this property in other properties'
            // documentation and requirements.
            let renameCrossReferences = function (property, name) {
                property.doc = property.doc.replace(new RegExp('`' + oldName + '`', 'g'), '`' + conventionOverride.name + '`');
                let requires = property.requires || [];
                for (let i = 0; i < requires.length; i++) {
                    if (requires[i] === oldName) {
                        property.requires[i] = conventionOverride.name;
                    }
                    if (typeof requires[i] !== 'string') {
                        _.forOwn(requires[i], function (values, name, require) {
                            if (name === oldName) {
                                require[conventionOverride.name] = values;
                                delete require[name];
                            }
                        });
                    }
                }
            };
            _.forOwn(spec[kind.replace(/^layout_/, 'paint_')], renameCrossReferences);
            _.forOwn(spec[kind.replace(/^paint_/, 'layout_')], renameCrossReferences);
        }
    })
});

String.prototype.wrap = function (cols, indent) {
	let wrapRe = new RegExp(`(.{1,${cols - indent}})(?: +|\n|$)`, "gm");
	return this.replace(wrapRe, "$1\n").replace(/\s+$/, "").indent(indent);
};

String.prototype.indent = function (cols) {
	return this.replace(/^|\n/g, "$&" + " ".repeat(cols));
};

global.camelize = function (str) {
    return str.replace(/(?:^|-)(.)/g, function (_, x) {
        return x.toUpperCase();
    });
};

global.camelizeWithLeadingLowercase = function (str) {
    return str.replace(/-(.)/g, function (_, x) {
        return x.toUpperCase();
    });
};

// Returns true only if property is an enum or if it is an array
// property with uniquely defined enum.
global.definesEnum = function(property, allProperties) {
    if (property.type === "enum") {
        return true;
    }

    if (property.type === 'array' && property.value === 'enum') {
        const uniqueArrayEnum = (prop, enums) => {
            if (prop.value !== 'enum') return false;
            const enumsEqual = (val1, val2) => val1.length === val1.length && val1.every((val, i) => val === val2[i]);
            return enums.filter(e => enumsEqual(Object.keys(prop.values).sort(), Object.keys(e.values).sort())).length == 0;
        };

        const allEnumProperties = _(allProperties).filter({'type': 'enum'}).value();
        const uniqueArrayEnumProperties = _(allProperties).filter({'type': 'array'}).filter(prop => uniqueArrayEnum(prop, allEnumProperties)).value();
        return _(uniqueArrayEnumProperties).filter({'name': property.name}).value().length != 0;
    }

    return false;
}

global.objCName = function (property) {
    return camelizeWithLeadingLowercase(property.name);
};

global.objCGetter = function (property) {
    return camelizeWithLeadingLowercase(property.getter || property.name);
};

global.objCType = function (layerType, propertyName) {
    return `${prefix}${camelize(propertyName)}`;
};

global.arrayType = function (property) {
    return property.type === 'array' ? originalPropertyName(property).split('-').pop() : false;
};

global.testImplementation = function (property, layerType, isFunction) {
    let helperMsg = testHelperMessage(property, layerType, isFunction);
    return `layer.${objCName(property)} = [MLNRuntimeStylingHelper ${helperMsg}];`;
};

global.objCTestValue = function (property, layerType, arraysAsStructs, indent) {
    let propertyName = originalPropertyName(property);

    const paddingTestValue = () => {
        if (arraysAsStructs) {
            let iosValue = '[NSValue valueWithUIEdgeInsets:UIEdgeInsetsMake(1, 1, 1, 1)]'.indent(indent * 4);
            let macosValue = '[NSValue valueWithEdgeInsets:NSEdgeInsetsMake(1, 1, 1, 1)]'.indent(indent * 4);
            return `@"%@",\n#if TARGET_OS_IPHONE\n${iosValue}\n#else\n${macosValue}\n#endif\n${''.indent((indent - 1) * 4)}`;
        }
        return '@"{1, 1, 1, 1}"';
    }

    switch (property.type) {
        case 'boolean':
            return property.default ? '@"false"' : '@"true"';
        case 'number':
            return '@"1"';
        case 'formatted':
            // Special 'string' case to handle constant expression text-field that automatically
            // converts Formatted back to string.
            return layerType === 'string' ?
                `@"'${_.startCase(propertyName)}'"` :
                `@"${_.startCase(propertyName)}"`;
        case 'resolvedImage':
            return layerType === 'string' ?
                `@"${_.startCase(propertyName)}"` :
                `@"MLN_FUNCTION('image', '${_.startCase(propertyName)}')"`;
        case 'string':
            return `@"'${_.startCase(propertyName)}'"`;
        case 'enum':
            return `@"'${_.last(_.keys(property.values))}'"`;
        case 'color':
        case 'color-ramp': // ADDED
            return '@"%@", [MLNColor redColor]';
        case 'padding':
            return paddingTestValue();
        case 'variableAnchorOffsetCollection':
            return `@"{"top": [1, 2]}"`;
        case 'array':
            switch (arrayType(property)) {
                case 'dasharray':
                    return '@"{1, 2}"';
                case 'font':
                    return `@"{'${_.startCase(propertyName)}', '${_.startCase(_.reverse(propertyName.split('')).join(''))}'}"`;
                case 'padding': {
                    return paddingTestValue();
                }
                case 'offset':
                case 'translate': {
                    if (arraysAsStructs) {
                        let iosValue = '[NSValue valueWithCGVector:CGVectorMake(1, 1)]'.indent(indent * 4);
                        let macosValue = '[NSValue valueWithMLNVector:CGVectorMake(1, -1)]'.indent(indent * 4);
                        return `@"%@",\n#if TARGET_OS_IPHONE\n${iosValue}\n#else\n${macosValue}\n#endif\n${''.indent((indent - 1) * 4)}`;
                    }
                    return '@"{1, 1}"';
                }
                case 'anchor':
                    return `@"{'top','bottom'}"`;
                case 'mode':
                    return `@"{'horizontal','vertical'}"`;
                default:
                    throw new Error(`unknown array type for ${property.name}`);
            }
        default:
            throw new Error(`unknown type for ${property.name}`);
    }
};

global.mbglTestValue = function (property, layerType) {
    let propertyName = originalPropertyName(property);
    switch (property.type) {
        case 'boolean':
            return property.default ? 'false' : 'true';
        case 'number':
            return '1.0';
        case 'formatted':
        case 'string':
        case 'resolvedImage':
        case 'variableAnchorOffsetCollection':
            return `"${_.startCase(propertyName)}"`;
        case 'enum': {
            let type = camelize(originalPropertyName(property));
            if (/-translate-anchor$/.test(originalPropertyName(property))) {
                type = 'TranslateAnchor';
            }
            if (/-(rotation|pitch)-alignment$/.test(originalPropertyName(property))) {
                type = 'Alignment';
            }
            if (/^(text|icon)-anchor$/.test(originalPropertyName(property))) {
                type = 'SymbolAnchor'
            }
            let value = camelize(_.last(_.keys(property.values)));
            if (property['light-property']) {
                return `mbgl::style::Light${type}Type::${value}`;
            }
            return `mbgl::style::${type}Type::${value}`;
        }
        case 'color':
            return 'mbgl::Color::red()';
        case 'padding':
            return 'mbgl::Padding{1, 1, 1, 1}';
        case 'array':
            switch (arrayType(property)) {
                case 'dasharray':
                    return '{{1.0, 2.0}}';
                case 'font':
                    return `{"${_.startCase(propertyName)}", "${_.startCase(_.reverse(propertyName.split('')).join(''))}"}`;
                case 'padding':
                    return 'mbgl::Padding{1, 1, 1, 1}';
                case 'offset':
                case 'translate':
                    return 'mbgl::TranslateAnchorType::Viewport';
                case 'anchor':
                    return 'mbgl::SymbolAnchorType::Top';
                case 'mode':
                    return '{{mbgl::style::WritingModeType::Horizontal, mbgl::style::WritingModeType::Vertical}}';
                default:
                    throw new Error(`unknown array type for ${property.name}`);
            }
        default:
            throw new Error(`unknown type for ${property.name}`);
    }
};

global.testHelperMessage = function (property, layerType, isFunction) {
    let fnSuffix = isFunction ? 'Function' : 'Constant';

    switch (property.type) {
        case 'boolean':
            return `testBool${fnSuffix}:@(${property.default ? 'false' : 'true'})`;
        case 'number':
            return `testNumber${fnSuffix}:@1`;
        case 'formatted':
            return `testFormatted${fnSuffix}:@"${_.startCase(originalPropertyName(property))}"`;
        case 'string':
        case 'resolvedImage':
            return `testString${fnSuffix}:@"${_.startCase(originalPropertyName(property))}"`;
        case 'enum': {
            let objCType = global.objCType(layerType, property.name);
            let objCEnum = `${objCType}${camelize(Object.keys(property.values)[Object.keys(property.values).length-1])}`;
            return `testEnum${fnSuffix}:${objCEnum} type:@encode(${objCType})`;
        }
        case 'color':
        case 'color-ramp': // ADDED
            return 'testColor' + fnSuffix;
        case 'padding':
            return 'testPaddingType' + fnSuffix;
        case 'variableAnchorOffsetCollection':
            return `testVariableAnchorOffsetCollection${fnSuffix}:@[[NSValue valueWithMLNTextAnchor:MLNTextAnchorTop], [NSValue valueWithCGVector:CGVectorMake(1, 2)]]`;
        case 'array':
            switch (arrayType(property)) {
                case 'dasharray':
                    return 'testDashArray' + fnSuffix;
                case 'font':
                    return 'testFont' + fnSuffix;
                case 'padding':
                    return 'testPadding' + fnSuffix;
                case 'offset':
                case 'translate':
                    return 'testOffset' + fnSuffix;
                default:
                    throw new Error(`unknown array type for ${property.name}`);
            }
        default:
            throw new Error(`unknown type for ${property.name}`);
    }
};

const describeValue = (value, property, layerType) => {
    const formatNumber = (num) => num.toLocaleString().replace('-', '\u2212');
    const describeArray = () => {
        const isOffset = arrayType(property) === 'offset';
        const isTranslate = arrayType(property) === 'translate';
        const isPadding = arrayType(property) === 'padding';
        const isRadial = arrayType(property) === 'radial';
        const isPosition = arrayType(property) === 'position';
        const isDashArray = arrayType(property) === 'dasharray';
        const isFont = arrayType(property) === 'font';
        const isMode = arrayType(property) === 'mode';
        if (isOffset || isTranslate) {
            let description = 'an `NSValue` object containing a `CGVector` struct set to' +
                ` a horizontal value of ${formatNumber(value[0])} and a vertical value of ${formatNumber(value[1])}`;
            if (property.units) {
                description += ` ${property.units.replace(/pixel/, 'point')}`;
            }
            return description;
        }
        if (isPadding) {
            let units = property.units || '';
            if (units) { units = ` ${units}`.replace(/pixel/, 'point'); }
            if (value.every(num => num === 0)) {
                return 'an `NSValue` object containing `UIEdgeInsetsZero`';
            }
            if (value.length === 1) {
                return 'an `NSValue` object containing a `UIEdgeInsets` struct set to' +
                    ` ${formatNumber(value[0])}${units} on all sides`;
            }
            return 'an `NSValue` object containing a `UIEdgeInsets` struct set to' +
                ` ${formatNumber(value[0])}${units} on the top, ${formatNumber(value[3])}${units} on the left, ${formatNumber(value[2])}${units} on the bottom, and ${formatNumber(value[1])}${units} on the right`;
        }
        if (isPosition || isRadial) {
            return 'an array of three numbers specifying a [radial, azimuthal, polar] position, such as' +
                ` ${formatNumber(value[0])} radial, ${formatNumber(value[1])} azimuthal and ${formatNumber(value[2])} polar`;
        }
        if (isDashArray) {
            return 'an array of numbers, measured in points, that specifies the lengths of the alternating dashes and gaps that make up the line';
        }
        if (isFont) {
            return 'an array of font names';
        }
        if (isMode) {
            return 'an array of text writing modes';
        }

        switch (arrayType(property)) {
            case 'dasharray':
                return 'the array `' + value.join('`, `') + '`';
            case 'font':
                return 'the array `' + value.map(val => `'${val}'`).join('`, `') + '`';
            case 'offset':
                return 'the array `' + value.join('`, `') + '`';
            case 'translate':
                return 'the array `' + value.join('`, `') + '`';
            case 'anchor':
                return 'the array `' + value.join('`, `') + '`';
            case 'mode':
                return 'the array `' + value.join('`, `') + '`';
            case 'padding':
                return 'the array `' + value.join('`, `') + '`';
            default:
                return 'the array `' + value.join('`, `') + '`';
        }
    };

    switch (property.type) {
        case 'boolean':
            return value ? '`YES`' : '`NO`';
        case 'number':
            return 'the float ' + '`' + formatNumber(value) + '`';
        case 'formatted':
        case 'string':
            return 'the string `' + value + '`';
        case 'resolvedImage':
            return 'the string `' + value + '`';
        case 'enum':
            return 'the enum value `' + global.objCType(layerType, property.name) + global.camelize(value) + '`';
        case 'color':
        case 'color-ramp':
            return 'the color `' + value + '`';
        case 'padding':
            return describeArray();
        case 'array':
            return describeArray();
        case 'variableAnchorOffsetCollection':
            return 'array of interleaved `MLNTextAnchor` and `CGVector` values';
        default:
            throw new Error(`unknown type for ${property.name}`);
    }
};

global.formatNumber = function (num) {
    return num.toLocaleString().replace('-', '\u2212');
}

global.propertyDefault = function (property, layerType) {
    if (property.name === 'heatmap-color') {
        return 'an expression that evaluates to a rainbow color scale from blue to red';
    } else {
        return 'an expression that evaluates to ' + describeValue(property.default, property, layerType);
    }
};

global.originalPropertyName = function (property) {
    return property.original || property.name;
};

global.enumName = function (property) {
    return property.enumName || property.name;
};

global.propertyType = function (property) {
    switch (property.type) {
        case 'boolean': return 'NSNumber *';
        case 'number': return 'NSNumber *';
        case 'formatted':
        case 'string':
        case 'resolvedImage': return 'NSString *';
        case 'enum': return 'NSValue *';
        case 'color':
        case 'color-ramp': // ADDED
            return 'MLNColor *';
        case 'padding': return 'NSValue *';
        case 'array':
            switch (arrayType(property)) {
                case 'dasharray': return 'NSArray<NSNumber *> *';
                case 'font': return 'NSArray<NSString *> *';
                case 'padding': return 'NSValue *';
                case 'offset':
                case 'translate': return 'NSValue *';
                case 'anchor':
                case 'mode': return 'NSValue *';
                default: throw new Error(`unknown array type for ${property.name}`);
            }
        case 'variableAnchorOffsetCollection': return 'NSArray<NSValue *> *';
        default: throw new Error(`unknown type for ${property.name}`);
    }
};

global.mbglType = function(property) {
    switch (property.type) {
        case 'boolean': return 'bool';
        case 'number': return 'float';
        case 'formatted': return 'mbgl::style::expression::Formatted';
        case 'resolvedImage': return 'mbgl::style::expression::Image';
        case 'string': return 'std::string';
        case 'enum': {
            let type = camelize(originalPropertyName(property));
            if (property['light-property']) {
                return `mbgl::style::Light${type}Type`;
            }
            if (/-translate-anchor$/.test(originalPropertyName(property))) {
                type = 'TranslateAnchor';
            }
            if (/-(rotation|pitch)-alignment$/.test(originalPropertyName(property))) {
                type = 'Alignment';
            }
            if (/^(text|icon)-anchor$/.test(originalPropertyName(property))) {
                type = 'SymbolAnchor'
            }
            return `mbgl::style::${type}Type`;
        }
        case 'color':
        case 'color-ramp': // ADDED
            return 'mbgl::Color';
        case 'padding': return 'mbgl::Padding';
        case 'variableAnchorOffsetCollection': return 'mbgl::VariableAnchorOffsetCollection';
        case 'array':
            switch (arrayType(property)) {
                case 'dasharray': return 'std::vector<float>';
                case 'font': return 'std::vector<std::string>';
                case 'padding': return 'mbgl::Padding';
                case 'offset':
                case 'translate': return 'mbgl::style::TranslateAnchorType';
                case 'anchor': return 'mbgl::style::SymbolAnchorType';
                case 'mode': return 'std::vector<mbgl::style::WritingModeType>';
                default: throw new Error(`unknown array type for ${property.name}`);
            }
        default: throw new Error(`unknown type for ${property.name}`);
    }
};

const root = path.join(import.meta.dirname, "../..")
const outLocation = args.values.out ? args.values.out : root;

const layerH = readAndCompile(`platform/darwin/src/MLNStyleLayer.h.ejs`, root);
const layerPrivateH = readAndCompile(`platform/darwin/src/MLNStyleLayer_Private.h.ejs`, root);
const layerM = readAndCompile(`platform/darwin/src/MLNStyleLayer.mm.ejs`, root);
const lightH = readAndCompile(`platform/darwin/src/MLNLight.h.ejs`, root);
const lightM = readAndCompile(`platform/darwin/src/MLNLight.mm.ejs`, root);

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

function duplicatePlatformDecls(src) {
    // Duplicate `MLNColor` declarations for `NSColor` on macOS.
    src = src.replace(/(\/\*\*(?:\*[^\/]|[^*])*?\b(?:MLN|NS|UI)Color\b[\s\S]*?\*\/)(\s*.+?;)/g,
                       (match, comment, decl) => {
        let macosComment = comment.replace(/\bcolor\b/g, 'color')
            .replace(/\bMLNColor\b/g, 'NSColor');
        return `\\
#if TARGET_OS_IPHONE
${comment}${decl}
#else
${macosComment}${decl}
#endif`;
    });

    // Duplicate `UIEdgeInsets` declarations for `NSEdgeInsets` on macOS.
    return src.replace(/(\/\*\*(?:\*[^\/]|[^*])*?\bUI(EdgeInsets(?:Zero)?)\b[\s\S]*?\*\/)(\s*.+?;)/g,
                       (match, comment, decl) => {
        let macosComment = comment.replace(/\bdownward\b/g, 'upward')
            .replace(/\bUI(EdgeInsets(?:Zero)?)\b/g, 'NS$1');
        return `\\
#if TARGET_OS_IPHONE
${comment}${decl}
#else
${macosComment}${decl}
#endif`;
    });
}

var renamedPropertiesByLayerType = {};

for (var layer of layers) {
    layer.properties = _.concat(layer.layoutProperties, layer.paintProperties);
    let enumProperties = _.filter(layer.properties, prop => definesEnum(prop, layer.properties));
    if (enumProperties.length) {
        layer.enumProperties = enumProperties;
    }

    let renamedProperties = {};
    _.assign(renamedProperties, _.filter(layer.properties, prop => 'original' in prop || 'getter' in prop));
    if (!_.isEmpty(renamedProperties)) {
        renamedPropertiesByLayerType[layer.type] = renamedProperties;
    }

    writeIfModified(`platform/darwin/src/${prefix}${camelize(layer.type)}${suffix}.h`,
        duplicatePlatformDecls(layerH(layer)), outLocation);
    writeIfModified(`platform/darwin/src/${prefix}${camelize(layer.type)}${suffix}_Private.h`,
        duplicatePlatformDecls(layerPrivateH(layer)),  outLocation);
    writeIfModified(`platform/darwin/src/${prefix}${camelize(layer.type)}${suffix}.mm`,
        layerM(layer), outLocation);
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

writeIfModified(`platform/darwin/src/${prefix}Light.h`,
    duplicatePlatformDecls(lightH({properties: lightProperties})), outLocation);
writeIfModified(`platform/darwin/src/${prefix}Light.mm`,
    lightM({properties: lightProperties, renamedPropertiesByLayerType}), outLocation);
