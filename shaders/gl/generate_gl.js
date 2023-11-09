'use strict';

const {
    HeaderComment,
    emitDecompressor,
    emitAmalgamationDecompressor,
    emitAccessor,
    emitSource,
    writeFile,
    vprint
} = require("../common.js");
const path = require("node:path");
const fs = require("node:fs")

const headerTemplate = (name, additionalIncludes, infoBlock, dataBlock) => {
    return `${HeaderComment}
#pragma once
#include <mbgl/shaders/shader_source.hpp>
${additionalIncludes}

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::${name}, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "${name}";
${infoBlock}

${dataBlock}
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
`;
};

const newAttribLocationMapping = (source) => {
    return {__alloc: findHighestAttribLocation(source)};
}

const locationForAttrib = (attribLocations, locationName) => {
    if (attribLocations[locationName]) {
        return attribLocations[attribLocations];
    }

    attribLocations[locationName] = ++ attribLocations.__alloc;
    return attribLocations[locationName];
};

const findHighestAttribLocation = (source) => {
    const re = /layout\s*\(\s*location\s*=\s*(\d+)\s*\)\s*in\s+/g;
    let match;
    let topIndex = -1;

    while (match = re.exec(source)) {
        topIndex = Number(match[1]) > topIndex ? Number(match[1]) : topIndex;
    }

    return topIndex;
}

const pragmaMapConvert = (source, pragmaMap, attribLocations, pipelineStage) => {
    const re = /#pragma mapbox: ([\w]+) ([\w]+) ([\w]+) ([\w]+)/g;

    if (pipelineStage == "fragment") {
        return source.replace(re, (match, operation, precision, type, name) => {
            pragmaMap[name] = true;
            if (operation === 'define') {
                return `#ifndef HAS_UNIFORM_u_${name}
in ${precision} ${type} ${name};
#else
uniform ${precision} ${type} u_${name};
#endif`;
            } else { /* if (operation === 'initialize') */
                return `#ifdef HAS_UNIFORM_u_${name}
${precision} ${type} ${name} = u_${name};
#endif`;
            }
        });
    }

    // else pipelineStage == "vertex"

    return source.replace(re, (match, operation, precision, type, name) => {
        const attrType = type === 'float' ? 'vec2' : 'vec4';
        const unpackType = name.match(/color/) ? 'color' : attrType;

        if (pragmaMap[name]) {
            if (operation === 'define') {
                return `#ifndef HAS_UNIFORM_u_${name}
uniform lowp float u_${name}_t;
layout (location = ${
                    locationForAttrib(attribLocations, name)
                }) in ${precision} ${attrType} a_${name};
out ${precision} ${type} ${name};
#else
uniform ${precision} ${type} u_${name};
#endif`;
            } else { /* if (operation === 'initialize') */
                if (unpackType === 'vec4') { // vec4 attributes are only used for cross-faded properties, and are not packed
                    return `#ifndef HAS_UNIFORM_u_${name}
${name} = a_${name};
#else
${precision} ${type} ${name} = u_${name};
#endif`;
                } else {
                    return `#ifndef HAS_UNIFORM_u_${name}
${name} = unpack_mix_${unpackType}(a_${name}, u_${name}_t);
#else
${precision} ${type} ${name} = u_${name};
#endif`;
                }
            }
        } else {
            if (operation === 'define') {
                return `#ifndef HAS_UNIFORM_u_${name}
uniform lowp float u_${name}_t;
layout (location = ${
                    locationForAttrib(attribLocations, name)
                }) in ${precision} ${attrType} a_${name};
#else
uniform ${precision} ${type} u_${name};
#endif`;
            } else { /* if (operation === 'initialize') */
                if (unpackType === 'vec4') { // vec4 attributes are only used for cross-faded properties, and are not packed
                    return `#ifndef HAS_UNIFORM_u_${name}
${precision} ${type} ${name} = a_${name};
#else
${precision} ${type} ${name} = u_${name};
#endif`;
                } else { /* */
                    return `#ifndef HAS_UNIFORM_u_${name}
${precision} ${type} ${name} = unpack_mix_${unpackType}(a_${name}, u_${name}_t);
#else
${precision} ${type} ${name} = u_${name};
#endif`;
                }
            }
        }
    });
};

// / This variant does not emit any uniforms and instead controls access to UBOs
const pragmaMapConvertOnlyVertexArrays = (source, pragmaMap, attribLocations, pipelineStage) => {
    const re = /#pragma mapbox: ([\w]+) ([\w]+) ([\w]+) ([\w]+)/g;

    if (pipelineStage == "fragment") {
        return source.replace(re, (match, operation, precision, type, name) => {
            pragmaMap[name] = true;
            if (operation === 'define') {
                return `#ifndef HAS_UNIFORM_u_${name}
in ${precision} ${type} ${name};
#endif`;
            } else { /* if (operation === 'initialize') */
                return `#ifdef HAS_UNIFORM_u_${name}
${precision} ${type} ${name} = u_${name};
#endif`;
            }
        });
    }

    // else pipelineStage == "vertex"

    return source.replace(re, (match, operation, precision, type, name) => {
        const attrType = type === 'float' ? 'vec2' : 'vec4';
        const unpackType = name.match(/color/) ? 'color' : attrType;

        if (pragmaMap[name]) {
            if (operation === 'define') {
                return `#ifndef HAS_UNIFORM_u_${name}
layout (location = ${
                    locationForAttrib(attribLocations, name)
                }) in ${precision} ${attrType} a_${name};
out ${precision} ${type} ${name};
#endif`;
            } else { /* if (operation === 'initialize') */
                if (unpackType === 'vec4') { // vec4 attributes are only used for cross-faded properties, and are not packed
                    return `#ifndef HAS_UNIFORM_u_${name}
${name} = a_${name};
#else
${precision} ${type} ${name} = u_${name};
#endif`;
                } else {
                    return `#ifndef HAS_UNIFORM_u_${name}
${name} = unpack_mix_${unpackType}(a_${name}, u_${name}_t);
#else
${precision} ${type} ${name} = u_${name};
#endif`;
                }
            }
        } else {
            if (operation === 'define') {
                return `#ifndef HAS_UNIFORM_u_${name}
layout (location = ${
                    locationForAttrib(attribLocations, name)
                }) in ${precision} ${attrType} a_${name};
#endif`;
            } else { /* if (operation === 'initialize') */
                if (unpackType === 'vec4') { // vec4 attributes are only used for cross-faded properties, and are not packed
                    return `#ifndef HAS_UNIFORM_u_${name}
${precision} ${type} ${name} = a_${name};
#else
${precision} ${type} ${name} = u_${name};
#endif`;
                } else { /* */
                    return `#ifndef HAS_UNIFORM_u_${name}
${precision} ${type} ${name} = unpack_mix_${unpackType}(a_${name}, u_${name}_t);
#else
${precision} ${type} ${name} = u_${name};
#endif`;
                }
            }
        }
    });
};

module.exports = function (outputRoot, args) {
    const shaderRoot = "shaders/gl";
    const glRoot = path.join(outputRoot.hdrs, "gl");

    console.log("Writing GL shaders to", glRoot);

    let srcMetrics = {
        "args": args,
        "sizeTotal": 0,
        "sizeReduced": 0,
        "amalgamationPtr": 0
    };
    let generatedHeaders = [];
    let shaderNames = [];
    let amalgamation = [];

    JSON.parse(fs.readFileSync(path.join(shaderRoot, "manifest.json"))).filter(it => typeof it == "object").forEach((elem) => {
        const shaderHeaderPath = path.join(glRoot, elem.header + ".hpp");
        const shaderVertPath = path.join(shaderRoot, elem.glsl_vert);
        const shaderFragPath = path.join(shaderRoot, elem.glsl_frag);

        // A program defined in the manifest may request additional headers be included for relevant metadata
        let additionalIncludes = (typeof elem.additionalIncludes == "object"
            ? elem.additionalIncludes
            : []
        ).map(path => `#include <${path}>`);

        const vertexSource = fs.readFileSync(shaderVertPath, {encoding: "utf8"});
        const fragmentSource = fs.readFileSync(shaderFragPath, {encoding: "utf8"});

        // Generate permutation preprocessor directives
        let pragmaMap = [];
        let attribMap = newAttribLocationMapping(vertexSource);
        const fragSrc = elem.uses_ubos
            ? pragmaMapConvertOnlyVertexArrays(fragmentSource, pragmaMap, attribMap, "fragment")
            : pragmaMapConvert(fragmentSource, pragmaMap, attribMap, "fragment");
        const vertSrc = elem.uses_ubos
            ? pragmaMapConvertOnlyVertexArrays(vertexSource, pragmaMap, attribMap, "vertex")
            : pragmaMapConvert(vertexSource, pragmaMap, attribMap, "vertex");

        // The accessor is how we retrieve the (possibly packed) program source.
        // We have two for OpenGL as we use separate vertex and fragment programs.
        let accessorFunc = [];
        let vertexAccessor, fragmentAccessor;
        if (args.compress) {
            if (args.amalgamate) {
                vertexAccessor = emitAmalgamationDecompressor(elem.name, "OpenGL", "vertex");
                fragmentAccessor = emitAmalgamationDecompressor(elem.name, "OpenGL", "fragment");
            } else {
                vertexAccessor = emitDecompressor(elem.name, "OpenGL", "vertex");
                fragmentAccessor = emitDecompressor(elem.name, "OpenGL", "fragment");
            }
        } else {
            vertexAccessor = emitAccessor(elem.name, "OpenGL", "vertex");
            fragmentAccessor = emitAccessor(elem.name, "OpenGL", "fragment");
        }

        accessorFunc.push(vertexAccessor.func);
        accessorFunc.push(fragmentAccessor.func);

        // Coalesce the set of additional includes to a unique collection
        additionalIncludes = ([...new Set(additionalIncludes.concat(vertexAccessor.deps).concat(fragmentAccessor.deps))]).join("\n");

        vprint(args, `Writing shader ${shaderHeaderPath}...`);

        if (!(args.amalgamate && args.compress)) { // Each shader is placed in it's own container
            writeFile(
                shaderHeaderPath,
                headerTemplate(
                    elem.name,
                    additionalIncludes,
                    [], /* infoBlock */
                    ([
                        emitSource(shaderVertPath, srcMetrics, vertSrc, "vertex"),
                        emitSource(shaderFragPath, srcMetrics, fragSrc, "fragment"),
                        accessorFunc.join("\n"),
                    ]).join("\n") /* dataBlock */
                ),
                args.dryrun
            );

        } else {
            // Each shader is stored in a single buffer (the amalgamation)
            // Shaders store a pointer and length into the decompressed amalgamation (which we compress later)
            const vsh = emitSource(shaderVertPath, srcMetrics, vertSrc, "vertex");
            const fsh = emitSource(shaderFragPath, srcMetrics, fragSrc, "fragment");

            writeFile(
                shaderHeaderPath,
                headerTemplate(
                    elem.name,
                    additionalIncludes,
                    ([
                        `static constexpr const size_t vertexAmalgamationOffset = ${srcMetrics.amalgamationPtr};`,
                        `static constexpr const size_t vertexAmalgamationLength = ${vsh.length};`,
                        `static constexpr const size_t fragmentAmalgamationOffset = ${srcMetrics.amalgamationPtr + vsh.length};`,
                        `static constexpr const size_t fragmentAmalgamationLength = ${fsh.length};`,
                    ]).map(elem => "    " + elem).join("\n"), /* infoBlock */
                    ([
                        accessorFunc.join("\n"),
                    ]).join("\n") /* dataBlock */
                ),
                args.dryrun
            );

            srcMetrics.amalgamationPtr += vsh.length + fsh.length;
            amalgamation.push(vsh);
            amalgamation.push(fsh);
        }

        generatedHeaders.push("#include <mbgl/shaders/gl/" + elem.header + ".hpp>");
        shaderNames.push(elem.name);
    }); // forEach

    return {
        "headers": generatedHeaders,
        "srcs": [], // GL has no source implementation currently
        "names": shaderNames,
        "metrics": srcMetrics,
        "amalgamation": amalgamation
    };
};
