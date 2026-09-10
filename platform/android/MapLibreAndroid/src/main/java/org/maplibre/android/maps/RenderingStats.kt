package org.maplibre.android.maps

/**
 * Rendering statistics of the map renderer.
 *
 * The fields of this class are populated by the native peer, they must not be renamed.
 */
class RenderingStats {
    /** Frame CPU encoding time (seconds) */
    @JvmField
    var encodingTime = 0.0

    /** Frame CPU rendering time (seconds) */
    @JvmField
    var renderingTime = 0.0

    /** Number of frames rendered */
    @JvmField
    var numFrames = 0

    /** Number of draw calls (`glDrawElements`, `drawIndexedPrimitives`, etc.) executed during the most recent frame */
    @JvmField
    var numDrawCalls = 0

    /** Total number of draw calls executed during all the frames */
    @JvmField
    var totalDrawCalls = 0

    /** Total number of textures created */
    @JvmField
    var numCreatedTextures = 0

    /** Net textures */
    @JvmField
    var numActiveTextures = 0

    /** Net texture bindings */
    @JvmField
    var numTextureBindings = 0

    /** Number of times a texture was updated */
    @JvmField
    var numTextureUpdates = 0

    /** Number of bytes used in texture updates */
    @JvmField
    var textureUpdateBytes = 0L

    /** Number of buffers created */
    @JvmField
    var totalBuffers = 0L

    /** Number of SDK-specific buffers created */
    @JvmField
    var totalBufferObjs = 0L

    /** Number of times a buffer is updated */
    @JvmField
    var bufferUpdates = 0L

    /** Number of times an SDK-specific buffer is updated */
    @JvmField
    var bufferObjUpdates = 0L

    /** Sum of update sizes */
    @JvmField
    var bufferUpdateBytes = 0L

    /** Number of active buffers */
    @JvmField
    var numBuffers = 0

    /** Number of active offscreen frame buffers */
    @JvmField
    var numFrameBuffers = 0

    /** Number of active index buffers */
    @JvmField
    var numIndexBuffers = 0

    /** Sum of index buffers update sizes */
    @JvmField
    var indexUpdateBytes = 0L

    /** Number of active vertex buffers */
    @JvmField
    var numVertexBuffers = 0

    /** Sum of vertex buffers update sizes */
    @JvmField
    var vertexUpdateBytes = 0L

    /** Number of active uniform buffers */
    @JvmField
    var numUniformBuffers = 0

    /** Number of times a uniform buffer is updated */
    @JvmField
    var numUniformUpdates = 0

    /** Sum of uniform buffers update sizes */
    @JvmField
    var uniformUpdateBytes = 0L

    /** Total texture memory */
    @JvmField
    var memTextures = 0

    /** Total buffer memory */
    @JvmField
    var memBuffers = 0

    /** Total index buffer memory */
    @JvmField
    var memIndexBuffers = 0

    /** Total vertex buffer memory */
    @JvmField
    var memVertexBuffers = 0

    /** Total uniform buffer memory */
    @JvmField
    var memUniformBuffers = 0

    /** Number of stencil buffer clears */
    @JvmField
    var stencilClears = 0

    /** Number of stencil buffer updates */
    @JvmField
    var stencilUpdates = 0
}
