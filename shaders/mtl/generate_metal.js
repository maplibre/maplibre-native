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
#include <mbgl/shaders/mtl/common.hpp>
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
        return {"hdr": "", "src": ""};
    }

    let metadata = []; // Decls placed in the header
    let srcImplData = [];

    // Implementation data place in the source file
    // attributes
    elem.metadata.attributes = typeof elem.metadata.attributes == "object" ? elem.metadata.attributes : [];
    elem.metadata.attributes = elem.metadata.attributes.filter(it => typeof it == "object");
    const numAttrs = elem.metadata.attributes.length;
    //if (numAttrs > 0) {
        // Declare this array in the header
        metadata.push(`    static const std::array<AttributeInfo, ${numAttrs}> attributes;`);

        // And define it in the source TLU
        srcImplData.push(`const std::array<AttributeInfo, ${numAttrs}> ShaderSource<BuiltIn::${
            elem.name
        }, gfx::Backend::Type::Metal>::attributes = {`);
        elem.metadata.attributes.forEach((attr) => {
            srcImplData.push(`    AttributeInfo{${
                attr.index
            }, gfx::AttributeDataType::${
                attr.type
            }, ${
                attr.count
            }, "${
                attr.name
            }"},`);
        });
        srcImplData.push("};");
    //}

    // uniforms
    elem.metadata.uniforms = typeof elem.metadata.uniforms == "object" ? elem.metadata.uniforms : [];
    elem.metadata.uniforms = elem.metadata.uniforms.filter(it => typeof it == "object");
    const numUniforms = elem.metadata.uniforms.length;
    //if (numUniforms > 0) {
        // Declare this array in the header
        metadata.push(`    static const std::array<UniformBlockInfo, ${numUniforms}> uniforms;`);

        // And define it in the source TLU
        srcImplData.push(`const std::array<UniformBlockInfo, ${numUniforms}> ShaderSource<BuiltIn::${
            elem.name
        }, gfx::Backend::Type::Metal>::uniforms = {`);
        elem.metadata.uniforms.forEach((uniform) => {
            srcImplData.push(`    UniformBlockInfo{${
                uniform.index
            }, ${
                uniform.bindVertex
            }, ${
                uniform.bindFragment
            }, sizeof(${
                uniform.structureName
            }), "${
                uniform.structureName
            }"},`);
        });
        srcImplData.push("};");
    //}

    // textures
    elem.metadata.textures = typeof elem.metadata.textures == "object" ? elem.metadata.textures : [];
    elem.metadata.textures = elem.metadata.textures.filter(it => typeof it == "object");
    const numTextures = elem.metadata.textures.length;
    //if (numTextures > 0) {
        // Declare this array in the header
        metadata.push(`    static const std::array<TextureInfo, ${numTextures}> textures;`);

        // And define it in the source TLU
        srcImplData.push(`const std::array<TextureInfo, ${numTextures}> ShaderSource<BuiltIn::${
            elem.name
        }, gfx::Backend::Type::Metal>::textures = {`);
        elem.metadata.textures.forEach((texture) => {
            srcImplData.push(`    TextureInfo{${
                texture.index
            }, "${
                texture.name
            }"},`);
        });
        srcImplData.push("};");
    //}

    // library links
    elem.program.link = typeof elem.program.link == "object" ? elem.program.link : [];
    elem.program.link = elem.program.link.filter(it => typeof it == "string");
    const numLinks = elem.program.link.length;
    //if (numLinks > 0) {
        // Declare this array in the header
        metadata.push(`    static const std::array<std::string_view, ${numLinks}> links;`);

        // And define it in the source TLU
        srcImplData.push(`const std::array<std::string_view, ${numLinks}> ShaderSource<BuiltIn::${
            elem.name
        }, gfx::Backend::Type::Metal>::links = {`);
        elem.program.link.forEach((link) => {
            srcImplData.push(`    std::string_view("${link}")},`);
        });
        srcImplData.push("};");
    //}

    return {"hdr": metadata.join("\n"), "src": srcImplData.join("\n")};
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

        vprint(args, `Writing shader ${shaderHeaderPath}...`);

        if (!(args.amalgamate && args.compress)) { // Each shader is placed in it's own container, possibly compressed
            writeFile(
                shaderHeaderPath,
                headerTemplate(
                    elem.name, /* shaderName */
                    additionalIncludes, ([
                        `static constexpr const char* name = "${elem.name}";`,
                        `static constexpr auto vertexMainFunction = "${elem.program.vertexEntry}";`,
                        `static constexpr auto fragmentMainFunction = "${elem.program.fragmentEntry}";`,
                    ]).map(elem => "    " + elem).join("\n"), /* infoBlock */
                    ([
                        reflectionData.hdr,
                        emitSource(programSrcPath, srcMetrics),
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
                        `static constexpr const char* name = "${elem.name}";`,
                        `static constexpr auto vertexMainFunction = "${elem.program.vertexEntry}";`,
                        `static constexpr auto fragmentMainFunction = "${elem.program.fragmentEntry}";`,
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
        if (reflectionData.src.length > 0) {
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
        "amalgamation": amalgamation
    };
};
