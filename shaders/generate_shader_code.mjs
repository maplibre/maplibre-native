console.log("Generating shaders...");

import { parseArgs } from "node:util";
import path from "node:path";
import fs from "node:fs";

const generatedHeader = `// Generated code, do not modify this file!`;

const newAttribLocationMapping = (source) => {
    return {
        __alloc: findHighestAttribLocation(source)
    };
}

const locationForAttrib = (attribLocations, locationName) => {
    if (attribLocations[locationName]) {
        return attribLocations[attribLocations];
    }

    attribLocations[locationName] = ++attribLocations.__alloc;
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
            } else /* if (operation === 'initialize') */ {
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
layout (location = ${locationForAttrib(attribLocations, name)}) in ${precision} ${attrType} a_${name};
out ${precision} ${type} ${name};
#else
uniform ${precision} ${type} u_${name};
#endif`;
            } else /* if (operation === 'initialize') */ {
                if (unpackType === 'vec4') {
                    // vec4 attributes are only used for cross-faded properties, and are not packed
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
layout (location = ${locationForAttrib(attribLocations, name)}) in ${precision} ${attrType} a_${name};
#else
uniform ${precision} ${type} u_${name};
#endif`;
            } else /* if (operation === 'initialize') */ {
                if (unpackType === 'vec4') {
                    // vec4 attributes are only used for cross-faded properties, and are not packed
                    return `#ifndef HAS_UNIFORM_u_${name}
${precision} ${type} ${name} = a_${name};
#else
${precision} ${type} ${name} = u_${name};
#endif`;
                } else /* */{
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

/// This variant does not emit any uniforms and instead controls access to UBOs
const pragmaMapConvertOnlyVertexArrays = (source, pragmaMap, attribLocations, pipelineStage) => {
    const re = /#pragma mapbox: ([\w]+) ([\w]+) ([\w]+) ([\w]+)/g;

    if (pipelineStage == "fragment") {
        return source.replace(re, (match, operation, precision, type, name) => {
            pragmaMap[name] = true;
            if (operation === 'define') {
                return `#ifndef HAS_UNIFORM_u_${name}
in ${precision} ${type} ${name};
#endif`;
            } else /* if (operation === 'initialize') */ {
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
layout (location = ${locationForAttrib(attribLocations, name)}) in ${precision} ${attrType} a_${name};
out ${precision} ${type} ${name};
#endif`;
            } else /* if (operation === 'initialize') */ {
                if (unpackType === 'vec4') {
                    // vec4 attributes are only used for cross-faded properties, and are not packed
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
layout (location = ${locationForAttrib(attribLocations, name)}) in ${precision} ${attrType} a_${name};
#endif`;
            } else /* if (operation === 'initialize') */ {
                if (unpackType === 'vec4') {
                    // vec4 attributes are only used for cross-faded properties, and are not packed
                    return `#ifndef HAS_UNIFORM_u_${name}
${precision} ${type} ${name} = a_${name};
#else
${precision} ${type} ${name} = u_${name};
#endif`;
                } else /* */{
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

const strip = (source) => {
    return source
        .replace(/^\s+/gm, "\n") // indentation, leading whitespace
        .replace(/\s*\/\/[^\n]*\n/g, "\n") // Single line comments
        .replace(/\n+\n/g, "\n") // extra new lines
        .replace(/\s?([+-\/<>*\?:=,])\s?/g, "$1") // whitespace around operators
        .replace(/([;\(\),\{\}])\n(?=[^#])/g, "$1"); // line breaks
};

// Parse command line
const args = parseArgs({
    options: {
        out: {
            type: 'string',
            short: 'o',
            description: 'Directory root to write generated code.'
        },
        compress: {
            type: 'string',
            short: 'c',
            description: 'Compress shader text with zlib and output byte arrays instead of strings'
        },
        strip: {
            type: 'boolean',
            short: 's',
            description: 'Strip comments, new lines and whitespace'
        }
    },
    allowPositionals: false
});


// Generate shader source headers
const root = path.dirname(import.meta.dirname);
const outLocation = args.out ? args.out : root;
const shaderRoot = path.join(root, "shaders");
const outputRoot = path.join(outLocation, "include/mbgl/shaders");
const cppOutputRoot = path.join(outLocation, "src/mbgl/shaders");
let generatedHeaders = [];
let shaderNames = [];

console.log("Writing shaders to ", outputRoot);

JSON.parse(fs.readFileSync(path.join(shaderRoot, "manifest.json")))
    .filter(it => typeof it == "object")
    .forEach((elem) => {
        const fragmentSource = fs.readFileSync(path.join(shaderRoot, elem.glsl_frag), {encoding: "utf8"});
        const vertexSource = fs.readFileSync(path.join(shaderRoot, elem.glsl_vert), {encoding: "utf8"});

        let pragmaMap = [];
        let attribMap = newAttribLocationMapping(vertexSource);

        const frag = elem.uses_ubos
            ? pragmaMapConvertOnlyVertexArrays(fragmentSource, pragmaMap, attribMap, "fragment")
            : pragmaMapConvert(fragmentSource, pragmaMap, attribMap, "fragment");
        const vert = elem.uses_ubos
            ? pragmaMapConvertOnlyVertexArrays(vertexSource, pragmaMap, attribMap, "vertex")
            : pragmaMapConvert(vertexSource, pragmaMap, attribMap, "vertex");

        const glRoot = path.join(outputRoot, "gl");
        if (!fs.existsSync(glRoot)) {
            fs.mkdirSync(glRoot, {recursive: true}); // Ensure target directory is available
        }

        fs.writeFileSync(
            path.join(glRoot, elem.header + ".hpp"),
            `${generatedHeader}
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::${elem.name}, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "${elem.name}";
    static constexpr const char* vertex = R"(${args.strip ? strip(vert) : vert})";
    static constexpr const char* fragment = R"(${args.strip ? strip(frag) : frag})";
};

} // namespace shaders
} // namespace mbgl
`);
        generatedHeaders.push("#include <mbgl/shaders/gl/" + elem.header + ".hpp>");
        shaderNames.push(elem.name);
    }
);

// Generate the manifest
fs.writeFileSync(path.join(outputRoot, "shader_manifest.hpp"),
`${generatedHeader}
#pragma once
#include <mbgl/shaders/shader_source.hpp>

#if MLN_RENDER_BACKEND_OPENGL
${generatedHeaders.join('\n')}
#endif
`);

// Generate shader_source.hpp
fs.writeFileSync(path.join(outputRoot, "shader_source.hpp"),
`${generatedHeader}
#pragma once
#include <mbgl/gfx/backend.hpp>

namespace mbgl {
namespace shaders {

/// @brief This enum is used with the ShaderSource template to select
/// source code for the desired program and graphics back-end.
enum class BuiltIn {
    None,
    ${shaderNames.join(',\n    ')}
};

/// @brief Select shader source based on a program type and a desired
/// graphics API.
/// @tparam T One of the built-in shader types available in the BuiltIn enum
/// @tparam The desired graphics API to request shader code for. One of
/// gfx::Backend::Type enums.
template <BuiltIn T, gfx::Backend::Type>
struct ShaderSource;

/// @brief A specialization of the ShaderSource template for no shader code.
template <>
struct ShaderSource<BuiltIn::None, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "";
    static constexpr const char* vertex = "";
    static constexpr const char* fragment = "";
};

} // namespace shaders
} // namespace mbgl
`);

// Generate shader_source.cpp
fs.writeFileSync(path.join(cppOutputRoot, "shader_source.cpp"),
`${generatedHeader}
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/util/enum.hpp>

namespace mbgl {

using namespace shaders;

MBGL_DEFINE_ENUM(BuiltIn, {
{BuiltIn::None, "None"},${shaderNames.map(name => `\n{BuiltIn::` + name + `, "` + name + `"}`)}
});

} // namespace mbgl
`);

console.log("Shaders generated!");
