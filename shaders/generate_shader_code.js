#!/usr/bin/env node
'use strict';

const generateGL = require("./gl/generate_gl.js");
const generateMetal= require("./mtl/generate_metal.js");
const { HeaderComment, percTrunc, writeFile, compress } = require("./common.js");
const { ArgumentParser } = require("argparse");
const path = require("node:path");
const fs = require("node:fs")
const zlib = require("zlib");

const generateBackendShaders = (generatorFunctor, backendName, outputRoot, args) => {
    console.log(`Generating ${backendName} shaders...`);
    const generated = generatorFunctor(outputRoot, args);
    console.log(
        `${backendName} Metrics: `
        + `\n\tTotal Size: ${percTrunc(generated.metrics.sizeTotal / 1024)}KiB`
        + `\n\tReduced Size: ${percTrunc(generated.metrics.sizeReduced / 1024)}KiB`);
    console.log("");
    return generated;
};

const writeShaderManifest = (outputRoot, generatedGLHeaders, generatedMetalHeaders, args) => {
    writeFile(path.join(outputRoot.hdrs, "shader_manifest.hpp"),
    `${HeaderComment}
#pragma once
#include <mbgl/shaders/shader_source.hpp>

${generatedGLHeaders.join('\n')}
${generatedMetalHeaders.join('\n')}

namespace mbgl {
namespace shaders {

struct ReflectionData;

/// @brief Get the name of the given shader ID as a string
std::string getProgramName(BuiltIn programID);

template <gfx::Backend::Type>
std::pair<std::string, std::string> getShaderSource(shaders::BuiltIn programID);

template <gfx::Backend::Type>
const ReflectionData& getReflectionData(BuiltIn programID);

} // namespace shaders
} // namespace mbgl

// NOLINTEND
`, args.dryrun);
};

// Generate shader_source.hpp
const writeShaderSource = (outputRoot, GL, MTL, args) => {
    let amalgamation = [];
    if (args.amalgamate && args.compress) {
        console.log("======= Amalgamation =======")
        // Build the amalgamation
        amalgamation.push("namespace amalgamation {");

        // GL
        if (args.opengl) {
            amalgamation.push("namespace OpenGL {");
            
            const glAmalgamation = GL.amalgamation.join("");
            const compressed = compress(glAmalgamation);
            console.log("GL Metrics:");
            console.log(`\tTotal Size: ${percTrunc(glAmalgamation.length / 1024)}KiB`);
            console.log(`\tReduced Size: ${percTrunc(compressed.len / 1024)}KiB`);
            amalgamation.push(`static constexpr const uint8_t data[] = {${compressed.arr}};`)
            amalgamation.push("} // namespace OpenGL ");
        }

        // Metal
        if (args.metal) {
            amalgamation.push("namespace Metal {");

            const mtlAmalgamation = MTL.amalgamation.join("");
            const compressed = compress(mtlAmalgamation);
            console.log("Metal Metrics:");
            console.log(`\tTotal Size: ${percTrunc(mtlAmalgamation.length / 1024)}KiB`);
            console.log(`\tReduced Size: ${percTrunc(compressed.len / 1024)}KiB`);
            amalgamation.push(`static constexpr const uint8_t data[] = {${compressed.arr}};`)
            amalgamation.push("} // namespace Metal ");
        }

        amalgamation.push("} // namespace amalgamation\n");
    }

    const programIDs = [...new Set(GL.names.concat(MTL.names))];

    writeFile(path.join(outputRoot.hdrs, "shader_source.hpp"),
    `${HeaderComment}
#pragma once
#include <mbgl/gfx/backend.hpp>
#include <string>
#include <utility>

${args.compress ? "#define MLN_COMPRESSED_SHADERS 1" : "#define MLN_COMPRESSED_SHADERS 0"}
${args.compress && args.amalgamate ? "#define MLN_AMALGAMATED_SHADERS 1" : "#define MLN_AMALGAMATED_SHADERS 0"}
namespace mbgl {
namespace shaders {

/// @brief This enum is used with the ShaderSource template to select
/// source code for the desired program and graphics back-end.
enum class BuiltIn : uint8_t {
    None = 0,
    ${(programIDs).join(",\n    ")}
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

template <>
struct ShaderSource<BuiltIn::None, gfx::Backend::Type::Metal> {
    static constexpr const char* name = "";
    static constexpr const char* vertexMainFunction = "";
    static constexpr const char* fragmentMainFunction = "";
    static constexpr const char* source = "";
};

${amalgamation.join("\n")}

} // namespace shaders
} // namespace mbgl

// NOLINTEND
`, args.dryrun);

    writeFile(path.join(outputRoot.srcs, "shader_manifest.cpp"),
    `${HeaderComment}
#include <mbgl/shaders/shader_manifest.hpp>
#include <type_traits>
#include <cassert>
#include <stdexcept>
#include <string>

namespace std {
template <>
struct hash<mbgl::shaders::BuiltIn> {
    constexpr size_t operator()(const mbgl::shaders::BuiltIn& id) const {
        return static_cast<size_t>(id);
    }
};
} // namespace std

#include <mapbox/eternal.hpp>

namespace mbgl {
namespace shaders {

constexpr const auto programNames = mapbox::eternal::hash_map<shaders::BuiltIn, mapbox::eternal::string>({
${(() => {
    let branches = [];
    programIDs.forEach((id) => {
        branches.push(`    { shaders::BuiltIn::${id}, "${id}" },`);
    });
    return branches.join("\n");
})()}
});

std::string getProgramName(shaders::BuiltIn programID) {
    const auto it = programNames.find(programID);
    assert(it != programNames.end());
    if (it == programNames.end()) {
        return "";
    }
    return it->second.c_str();
}

#if MLN_RENDER_BACKEND_OPENGL
template<BuiltIn ID>
using GLProgram = mbgl::shaders::ShaderSource<ID, gfx::Backend::Type::OpenGL>;

template <>
std::pair<std::string, std::string> getShaderSource<gfx::Backend::Type::OpenGL>(shaders::BuiltIn programID) {
    switch (programID) {
${(() => {
    let branches = [];
    programIDs.forEach((id) => {
        if (GL.names.includes(id)) {
            branches.push(`        case BuiltIn::${id}: { return std::make_pair(GLProgram<BuiltIn::${id}>::vertex(), GLProgram<BuiltIn::${id}>::fragment()); }`);
        }
    });
    return branches.join("\n");
})()}
        default: {
            return {"", ""};
        }
    }
}
#endif

#if MLN_RENDER_BACKEND_METAL
template<BuiltIn ID>
using MtlProgram = mbgl::shaders::ShaderSource<ID, gfx::Backend::Type::Metal>;

template <>
std::pair<std::string, std::string> getShaderSource<gfx::Backend::Type::Metal>(shaders::BuiltIn programID) {
    switch (programID) {
${(() => {
    let branches = [];
    programIDs.forEach((id) => {
        if (MTL.names.includes(id)) {
            branches.push(`        case BuiltIn::${id}: { return std::make_pair(MtlProgram<BuiltIn::${id}>::source(), ""); }`);
        }
    });
    return branches.join("\n");
})()}
        default: {
            throw std::runtime_error("No source found for program!");
        }
    }
}

template <>
const ReflectionData& getReflectionData<gfx::Backend::Type::Metal>(shaders::BuiltIn programID) {
    switch (programID) {
${(() => {
    let branches = [];
    programIDs.forEach((id) => {
        if (MTL.names.includes(id) && MTL.reflectedPrograms[id]) {
            branches.push(`        case BuiltIn::${id}: { return MtlProgram<BuiltIn::${id}>::reflectionData; }`);
        }
    });
    return branches.join("\n");
})()}
        default: {
            throw std::runtime_error("No reflection data found for program!");
        }
    }
}
#endif

} // namespace shaders
} // namespace mbgl

// NOLINTEND
`, args.dryrun);
};

const main = () => {
    // Parse command line
    const args = (() => {
        const parser = new ArgumentParser({
            description: "MapLibre Shader Tools",
            epilog: "Example invocation: node shaders/generate_shader_code.js --root ./ --opengl --metal --verbose --strip --compress --amalgamate"
        });
        parser.add_argument("--root", "--r", {
            help: "Directory root to place generated code",
            required: true
        });
        parser.add_argument("--compress", "--c", {
            help: "Compress shader text with zlib and output byte arrays instead of strings",
            required: false,
            action: "store_true"
        });
        parser.add_argument("--amalgamate", "--a", {
            help: "If using compression, pack everything in a unified compressed blob",
            required: false,
            action: "store_true"
        });
        parser.add_argument("--strip", "--s", {
            help: "Strip comments, new lines and whitespace",
            required: false,
            action: "store_true"
        });
        parser.add_argument("--opengl", "--gl", {
            help: "Write shaders for the OpenGL backend.",
            required: false,
            action: "store_true"
        });
        parser.add_argument("--metal", "--mtl", {
            help: "Write shaders for the Metal backend.",
            required: false,
            action: "store_true"
        });
        parser.add_argument("--verbose", "--d", {
            help: "Perform all steps but don't write anything to disk.",
            required: false,
            action: "store_true"
        });
        parser.add_argument("--dryrun", "--v", {
            help: "Print extra process information.",
            required: false,
            action: "store_true"
        });
        return parser.parse_args();
    })();

    if (args.opengl && args.metal) {
        console.log("Warning: Generating shaders for an unused backend may increase binary size.");
    }

    const outputRoot = {
        "hdrs": path.join(args.root, "include/mbgl/shaders"),
        "srcs": path.join(args.root, "src/mbgl/shaders"),
    };
    const emptyBackend = {"headers": [], "names": [], "amalgamation": []};
    const GL = args.opengl ? generateBackendShaders(generateGL, "OpenGL", outputRoot, args) : emptyBackend;
    const MTL = args.metal ? generateBackendShaders(generateMetal, "Metal", outputRoot, args) : emptyBackend;

    writeShaderManifest(
        outputRoot,
        ["#if MLN_RENDER_BACKEND_OPENGL"].concat(GL.headers).concat(["#endif"]),
        ["#if MLN_RENDER_BACKEND_METAL"].concat(MTL.headers).concat(["#endif"]),
        args
    );
    writeShaderSource(outputRoot, GL, MTL, args);
};
main();
