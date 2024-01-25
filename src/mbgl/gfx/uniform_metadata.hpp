#pragma once

#if MLN_DRAWABLE_RENDERER
// #ifndef NDEBUG

#include <map>
#include <string>
#include <vector>

namespace mbgl {
namespace uniform_metadata {

using UniformFieldMetadata = struct {
    std::size_t offset{0}, size{0};
};
using UniformBlockMetadata = std::map<std::string /* field name */, UniformFieldMetadata>;
using UniformsMetadata = std::map<std::string /* uniform name */, UniformBlockMetadata>;

void getChangedUBOFields(const std::string& name,
                         std::size_t size,
                         const uint8_t* before,
                         const uint8_t* after,
                         /*out*/ std::vector<std::string>& fieldNames,
                         /*out*/ std::size_t& updatedSize);
void getUBOFields(const std::string& name,
                  std::size_t size,
                  /*out*/ std::vector<std::string>& fieldNames,
                  /*out*/ std::size_t& dataSize);
} // namespace uniform_metadata
} // namespace mbgl

// #endif // !NDEBUG
#endif // MLN_DRAWABLE_RENDERER
