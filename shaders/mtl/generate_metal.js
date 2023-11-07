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
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <string_view>
${additionalIncludes}

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::${name}, gfx::Backend::Type::Metal> {
${infoBlock}

${dataBlock}
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
`;
};

const srcTemplate = (headers, reflectionData) => {
    return `${HeaderComment}
${headers}

namespace mbgl {
namespace shaders {

${reflectionData}

} // namespace shaders
} // namespace mbgl

// NOLINTEND
`;
};

// return reflection data provided in a manifest element's entry
const createReflection = ((elem) => {
    if (!elem.metadata) {
        return {"hdr": "", "src": "", "reflected": false};
    }

    let metadata = []; // Decls placed in the header
    let srcImplData = [];

    // Implementation data place in the source file
    // attributes
    elem.metadata.attributes = typeof elem.metadata.attributes == "object" ? elem.metadata.attributes : [];
    elem.metadata.attributes = elem.metadata.attributes.filter(it => typeof it == "object");
    const numAttrs = elem.metadata.attributes.length;

    // uniforms
    elem.metadata.uniforms = typeof elem.metadata.uniforms == "object" ? elem.metadata.uniforms : [];
    elem.metadata.uniforms = elem.metadata.uniforms.filter(it => typeof it == "object");
    const numUniforms = elem.metadata.uniforms.length;

    // textures
    elem.metadata.textures = typeof elem.metadata.textures == "object" ? elem.metadata.textures : [];
    elem.metadata.textures = elem.metadata.textures.filter(it => typeof it == "object");
    const numTextures = elem.metadata.textures.length;

    // library links
    elem.program.link = typeof elem.program.link == "object" ? elem.program.link : [];
    elem.program.link = elem.program.link.filter(it => typeof it == "string");
    const numLinks = elem.program.link.length;

    // runtime reflection data
    metadata.push(`    static const ReflectionData reflectionData;`);
    srcImplData.push(`const ReflectionData ShaderSource<BuiltIn::${elem.name}, gfx::Backend::Type::Metal>::reflectionData = {`);
    srcImplData.push(`    "${elem.name}",`,);
    srcImplData.push(`    "${elem.program.vertexEntry}",`);
    srcImplData.push(`    "${elem.program.fragmentEntry}",`);

    // attributes
    srcImplData.push(`    {
${(() => {
    let attributes = [];
    elem.metadata.attributes.forEach((attr) => {
        attributes.push(`        AttributeInfo{${attr.index}, gfx::AttributeDataType::${attr.type}, ${attr.count}, "${attr.name}"},`);
    });
    return attributes.join("\n");
})()}
    },`);

    // uniforms
    srcImplData.push(`    {
${(() => {
    let uniforms = [];
    elem.metadata.uniforms.forEach((uniform) => {
        uniforms.push(`        UniformBlockInfo{${uniform.index}, ${uniform.bindVertex}, ${uniform.bindFragment}, sizeof(${uniform.structureName}), "${uniform.structureName}"},`);
    });
    return uniforms.join("\n");
})()}
    },`);
    
    // textures
    srcImplData.push(`    {
${(() => {
    let textures = [];
    elem.metadata.textures.forEach((texture) => {
        textures.push(`        TextureInfo{${texture.index}, "${texture.name}"},`);
    });
    return textures.join("\n");
})()}
    },`);

    // links
    srcImplData.push(`    {
${(() => {
    let links = [];
    elem.program.link.forEach((link) => {
        links.push(`        BuiltIn::${link},`);
    });
    return links.join("\n");
})()}
    },`);
    srcImplData.push(`};`);

    return {"hdr": metadata.join("\n"), "src": srcImplData.join("\n"), "reflected": true};
});

module.exports = function (outputRoot, args) {
    const shaderRoot = "shaders/mtl";
    const mtlHdrRoot = path.join(outputRoot.hdrs, "mtl");
    const mtlSrcRoot = path.join(outputRoot.srcs, "mtl");

    console.log("Writing metal shaders to (hdr, src)", mtlHdrRoot, mtlSrcRoot);

    let srcMetrics = {
        "args": args,
        "sizeTotal": 0,
        "sizeReduced": 0,
        "amalgamationPtr": 0
    };
    let generatedHeaders = [];
    let generatedSources = [];
    let shaderNames = [];
    let reflectedPrograms = {};
    let amalgamation = [];

    JSON.parse(fs.readFileSync(path.join(shaderRoot, "manifest.json"))).filter(it => typeof it == "object").forEach((elem) => {
        const shaderHeaderPath = path.join(mtlHdrRoot, elem.header + ".hpp");
        const shaderSourcePath = path.join(mtlSrcRoot, elem.header + ".cpp");
        const programSrcPath = path.join(shaderRoot, elem.program.source);

        // A program defined in the manifest may request additional headers be included for relevant metadata
        let additionalIncludes = (typeof elem.additionalIncludes == "object" ? elem.additionalIncludes : []).map(path => `#include <${path}>`);

        // The accessor is how we retrieve the (possibly packed) program source
        let accessor;
        if (args.compress) {
            if (args.amalgamate) {
                accessor = emitAmalgamationDecompressor(elem.name, "Metal");
            } else {
                accessor = emitDecompressor(elem.name, "Metal");
            }
        } else {
            accessor = emitAccessor(elem.name, "Metal");
        }

        // Coalesce the set of additional includes to a unique collection
        additionalIncludes = ([...new Set(additionalIncludes.concat(accessor.deps))]).join("\n");
        const accessorFunc = accessor.func;
        const reflectionData = createReflection(elem);
        reflectedPrograms[elem.name] = reflectionData.reflected;

        vprint(args, `Writing shader ${shaderHeaderPath}...`);

        if (!(args.amalgamate && args.compress)) { // Each shader is placed in it's own container, possibly compressed
            writeFile(
                shaderHeaderPath,
                headerTemplate(
                    elem.name, /* shaderName */
                    additionalIncludes,
                    "", /* infoBlock */
                    ([
                        reflectionData.hdr,
                        emitSource(programSrcPath, srcMetrics, false, "source"),
                        accessorFunc,
                    ]).join("\n") /* dataBlock */
                ),
                args.dryrun
            );

        } else {
            // Each shader is stored in a single buffer (the amalgamation)
            // Shaders store a pointer and length into the decompressed amalgamation (which we compress later)
            const src = emitSource(programSrcPath, srcMetrics);

            writeFile(
                shaderHeaderPath,
                headerTemplate(
                    elem.name, /* shaderName */
                    additionalIncludes, ([
                        `static constexpr size_t sourceAmalgamationOffset = ${srcMetrics.amalgamationPtr};`,
                        `static constexpr size_t sourceAmalgamationLength = ${srcMetrics.amalgamationPtr + src.length};`,
                    ]).map(elem => "    " + elem).join("\n"), /* infoBlock */
                    ([
                        reflectionData.hdr,
                        accessorFunc,
                    ]).join("\n") /* dataBlock */
                ),
                args.dryrun
            );

            srcMetrics.amalgamationPtr += src.length;
            amalgamation.push(src);
        }

        const header = "#include <mbgl/shaders/mtl/" + elem.header + ".hpp>";

        // Write the source contents
        if (reflectionData.reflected && reflectionData.src.length > 0) {
            writeFile(shaderSourcePath, srcTemplate(header, reflectionData.src), args.dryrun);
        } else {
            vprint(args, "Not writing source file for shader " + elem.name + ", no metadata to define.");
        }

        generatedHeaders.push(header);
        generatedSources.push(shaderSourcePath)
        shaderNames.push(elem.name);
    }); // forEach

    return {
        "headers": generatedHeaders,
        "srcs": generatedSources,
        "names": shaderNames,
        "metrics": srcMetrics,
        "reflectedPrograms": reflectedPrograms,
        "amalgamation": amalgamation
    };
};
