#pragma once

namespace mbgl {
namespace style {

/**
 * Base parameters passed to CustomLayerHost::initialize().
 * Backend-specific subclasses provide device handles needed for
 * resource creation (pipelines, buffers, etc.).
 */
struct CustomLayerInitParameters {
    virtual ~CustomLayerInitParameters() = default;
};

} // namespace style
} // namespace mbgl
