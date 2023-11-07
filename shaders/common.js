'use strict';

const zlib = require("zlib");
const fs = require("node:fs")
const path = require ("node:path");

// Header inserted into every generated file
const HeaderComment = `// Generated code, do not modify this file!
// NOLINTBEGIN`;

// Truncate percentages to the 100th percentile.
const percTrunc = ((num) => {
    return Math.round(num * 100) * 0.01;
});

// Strip out whitespace, formatting and comments.
const strip = (source) => {
    return source
        .replace(/^\s+/gm, "\n") // indentation, leading whitespace
        .replace(/\s*\/\/[^\n]*\n/g, "\n") // Single line comments
        .replace(/\/\*.*\*\//gs, "\n") // Multi-line comments
        .replace(/\n+\n/g, "\n") // extra new lines
        .replace(/\s?([+-\/<>*\?:=,])\s?/g, "$1") // whitespace around operators
        .replace(/\s+(=)/g, "$1") // whitespace around operators
        .replace(/(,)\s+/g, "$1") // whitespace around operators
        .replace(/([;\(\),\{\}])\n(?=[^#])/g, "$1"); // line breaks
};

// Emit a `source()` accessor which reads from a compressed block.
const emitDecompressor = (shaderEnum, backend, name) => {
    name = typeof name == "undefined" ? "source" : name;
    const data = typeof name == "undefined" ? "data" : name + "Data";
    return {
        "deps": ["#include <mbgl/util/compression.hpp>"],
        "func": [
            `    static std::string ${name}() {`,
            `        using Ty = ShaderSource<BuiltIn::${shaderEnum}, gfx::Backend::Type::${backend}>;`,
            `        return mbgl::util::decompress(Ty::${data}, sizeof(Ty::${data}));`,
            "    }"
        ].join("\n")
    };
};

// Emit a `source()` accessor which reads from a compressed amalgamation.
// Using an amalgamation offers the best possible compression ratio, but does so
// by trading access performance.
const emitAmalgamationDecompressor = (shaderEnum, backend, name) => {
    name = typeof name == "undefined" ? "source" : name;
    return {
        "deps": ["#include <mbgl/util/compression.hpp>"],
        "func": [
            `    static std::string ${name}() {`,
            `        using Ty = ShaderSource<BuiltIn::${shaderEnum}, gfx::Backend::Type::${backend}>;`,
            `        const auto data = mbgl::util::decompress(mbgl::shaders::amalgamation::${backend}::data, sizeof(mbgl::shaders::amalgamation::${backend}::data));`,
            `        const auto str = data.substr(Ty::${name}AmalgamationOffset, Ty::${name}AmalgamationLength);`,
            `        return str;`,
            "    }"
        ].join("\n")
    };
};

// Emit a `source()` accessor which returns from an uncompressed string.
const emitAccessor = (shaderEnum, backend, name) => {
    name = typeof name == "undefined" ? "source" : name;
    const data = typeof name == "undefined" ? "data" : name + "Data";
    return {
        "deps": [],
        "func": [
            `    static std::string ${name}() {`,
            `        using Ty = ShaderSource<BuiltIn::${shaderEnum}, gfx::Backend::Type::${backend}>;`,
            `        return Ty::${data};`,
            "    }"
        ].join("\n")
    };
};

// Emit source code (possibly compressed).
/// sourceProvided: If not present, read source from disk specified by shaderSrcPath
/// name: If present, generate accessors prefixed with this name. ex: vertex, fragment
const emitSource = ((shaderSrcPath, srcMetrics, sourceProvided, name) => {
    name = typeof name == "undefined" ? "data" : name + "Data";
    let source = sourceProvided ? sourceProvided : fs.readFileSync(shaderSrcPath, {encoding: "utf8"});
    const orgSize = source.length;
    srcMetrics.sizeTotal += orgSize;

    let reduced = false;
    if (srcMetrics.args.strip) {
        source = strip(source);
        if (!srcMetrics.args.compress) {
            srcMetrics.sizeReduced += source.length;
            reduced = true;
            vprint(srcMetrics.args, `Stripped ${shaderSrcPath} to ${source.length} bytes - ${percTrunc((source.length / orgSize) * 100)}%`);
        }
    }

    if (srcMetrics.args.compress) {
        if (srcMetrics.args.amalgamate) {
            // The amalgamation is compressed as a single block, just return the processed source
            if (!reduced) {
                srcMetrics.sizeReduced += source.length;
            }

            return source;

        } else {
            source = compress(source);
            srcMetrics.sizeReduced += source.len;
            reduced = true;
            vprint(srcMetrics.args, `Compressed ${shaderSrcPath} to ${source.len} bytes - ${percTrunc((source.len / orgSize) * 100)}%`);
            return `    static constexpr const uint8_t ${name}[] = {${source.arr}};`;
        }
    } else {
        if (!reduced) {
            srcMetrics.sizeReduced += source.length;
        }

        return `    static constexpr const char* ${name} = R"(${source})";`;
    }
});

// Emit a compressed byte array for the given source string, using DEFLATE.
const compress = (source) => {
    const compressed = zlib.deflateSync(source, {level: 9});
    return {
        "len": compressed.length,
        "arr": ([...compressed].map(chr => {
            return "0x" + chr.toString(16);
        })).join(",")
    };
};

// Write a file, ensuring the path exists
// Accepts `dryRun` to perform a no-op.
const writeFile = (filePath, source, dryRun) => {
    if (dryRun) {
        return;
    }

    const folder = path.dirname(filePath);
    if (!fs.existsSync(folder)) {
        fs.mkdirSync(folder, {recursive: true}); // Ensure target directory is available
    }

    fs.writeFileSync(filePath, source, {flag: "w"});
};

// Print a string if the verbose parameter is set
const vprint = (args, str) => {
    if (args.verbose) {
        console.log(str);
    }
};

module.exports = {
    HeaderComment: HeaderComment,
    percTrunc: percTrunc,
    strip: strip,
    emitDecompressor: emitDecompressor,
    emitAmalgamationDecompressor: emitAmalgamationDecompressor,
    emitAccessor: emitAccessor,
    emitSource: emitSource,
    compress: compress,
    writeFile: writeFile,
    vprint: vprint,
};
