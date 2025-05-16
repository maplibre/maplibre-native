#pragma once

#include <cstddef>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#include <mbgl/gfx/types.hpp>
#include <mbgl/gl/types.hpp>
#include <mbgl/util/lru_cache.hpp>
#include <mbgl/util/size.hpp>

namespace mbgl {
namespace gl {

class Context;

struct Texture2DDesc {
    Size size;
    gfx::TexturePixelType pixelFormat;
    gfx::TextureChannelDataType channelType;

    size_t channelCount() const;
    size_t channelStorageSize() const;
    size_t getStorageSize() const;
};

struct Texture2DDescHash {
    std::size_t operator()(const Texture2DDesc& desc) const;
};

inline bool operator==(const Texture2DDesc& desc1, const Texture2DDesc& desc2) {
    return desc1.size == desc2.size && desc1.pixelFormat == desc2.pixelFormat && desc1.channelType == desc2.channelType;
}

inline bool operator!=(const Texture2DDesc& desc1, const Texture2DDesc& desc2) {
    return !(desc1 == desc2);
}

class Texture2DPool {
public:
    // Create a texture pool with a maximum storage size maxPoolStorage in bytes
    // If maxPoolStorage is 0, pooling will disabled
    Texture2DPool(Context* context, size_t maxPoolStorage = 16 * 1024 * 1024);

    // Deallocate all storage
    ~Texture2DPool();

    // Allocate a texture with the given size, pixel format, and channel type
    // If the texture is already allocated in the pool, the existing texture ID will be returned
    TextureID alloc(const Size& size, gfx::TexturePixelType pixelFormat, gfx::TextureChannelDataType channelType) {
        return alloc({size, pixelFormat, channelType});
    }

    TextureID alloc(const Texture2DDesc& desc);

    // Release the texture but keep it in the pool
    void release(TextureID id);

    // Free all unused textures
    void shrink();

    // Get the description of a texture
    const Texture2DDesc& desc(TextureID id) const;

    // Get the storage size in byte of a texture
    size_t storage(TextureID id) const;

    // Get the storage size in byte of all textures being used
    size_t usedStorage() const;

    // Get the storage size in byte of all unused textures
    size_t unusedStorage() const;

    // Get the storage size in byte of all textures in the pool
    size_t storage() const { return poolStorage; }

    // Get the maximum storage size before starting to evict unused textures
    size_t maxStorage() const { return maxPoolStorage; }

    bool empty() const { return poolStorage == 0; }

    // Check if a texture is pooled
    bool isPooled(TextureID id) const;

    // Check if a texture is pooled and being used
    bool isUsed(TextureID id) const;

    // Check if a texture is pooled but released
    bool isUnused(TextureID id) const;

private:
    struct TextureSet {
        std::unordered_set<TextureID> free;
        std::unordered_set<TextureID> used;
    };

    // Effectively glGenTextures and glTexImage2D
    TextureID allocateGLMemory(const Texture2DDesc& desc);

    // Effectively call glDeleteTextures
    void freeAllocatedGLMemory(TextureID id);

    // Evict the least recently used textures until the pool storage is below maxPoolStorage
    void evict();

private:
    Context* context = nullptr;
    size_t maxPoolStorage = 0;
    std::size_t poolStorage = 0;
    std::unordered_map<Texture2DDesc, TextureSet, Texture2DDescHash> pool;
    std::unordered_map<TextureID, Texture2DDesc> descriptions;
    LRU<TextureID> lru_cache;
    // stats
    int64_t allocCount = 0;   // total allocations
    int64_t reuseCount = 0;   // total reused allocations
    int64_t releaseCount = 0; // total textures released
    int64_t freedCount = 0;   // total textures released and deallocated
};

} // namespace gl
} // namespace mbgl
