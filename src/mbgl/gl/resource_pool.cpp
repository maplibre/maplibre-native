#include <mbgl/gl/resource_pool.hpp>

#include <cassert>
#include <type_traits>

#include <mbgl/gl/context.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/enum.hpp>
#include <mbgl/util/hash.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace gl {

using namespace platform;

namespace {

#if !defined(NDEBUG)
void logDebugMessage(const std::string& message) {
    Log::Debug(Event::General, "Texture2DPool: " + message);
}
#else
void logDebugMessage(const std::string&) {}
#endif

} // namespace

size_t Texture2DDesc::channelCount() const {
    switch (pixelFormat) {
        case gfx::TexturePixelType::Alpha:
        case gfx::TexturePixelType::Depth:
        case gfx::TexturePixelType::Luminance:
        case gfx::TexturePixelType::Stencil:
            return 1;
        case gfx::TexturePixelType::RGBA:
            return 4;
        default:
            assert(!"Unknown texture pixel type");
            return 0;
    }
}

size_t Texture2DDesc::channelStorageSize() const {
    switch (channelType) {
        case gfx::TextureChannelDataType::HalfFloat:
            return 2;
        case gfx::TextureChannelDataType::UnsignedByte:
            return 1;
        default:
            assert(!"Unknown texture channel data type");
            return 0;
    }
}

size_t Texture2DDesc::getStorageSize() const {
    return size.width * size.height * channelCount() * channelStorageSize();
}

std::size_t Texture2DDescHash::operator()(const Texture2DDesc& desc) const {
    std::size_t seed = 0;
    util::hash_combine(seed, desc.size.width);
    util::hash_combine(seed, desc.size.height);
    util::hash_combine(seed, std::underlying_type_t<gfx::TexturePixelType>(desc.pixelFormat));
    util::hash_combine(seed, std::underlying_type_t<gfx::TextureChannelDataType>(desc.channelType));
    return seed;
}

Texture2DPool::Texture2DPool(Context* context_, size_t maxPoolStorage_)
    : context(context_),
      maxPoolStorage(maxPoolStorage_) {
    assert(context);
}

Texture2DPool::~Texture2DPool() {
    assert(usedStorage() == 0);
    assert(unusedStorage() == poolStorage);
    shrink();
    assert(usedStorage() == 0);
    assert(unusedStorage() == 0);
    assert(poolStorage == 0);
    assert(lru_cache.empty());
    assert(descriptions.empty());
    assert(pool.empty());
    logDebugMessage("total allocations " + util::toString(allocCount));
    logDebugMessage("total reused allocations " + util::toString(reuseCount));
    logDebugMessage("total textures released " + util::toString(releaseCount));
    logDebugMessage("total textures released and deallocated " + util::toString(freedCount));
}

TextureID Texture2DPool::alloc(const Texture2DDesc& desc) {
    MLN_TRACE_FUNC();

    context->renderingStats().numActiveTextures++;

    // Check if the texture is already allocated
    auto set_it = pool.find(desc);
    if (set_it != pool.end() && !set_it->second.free.empty()) {
        // Reuse an existing texture
        TextureID id = *set_it->second.free.begin();
        set_it->second.free.erase(id);
        set_it->second.used.insert(id);
        assert(lru_cache.isHit(id));
        lru_cache.remove(id);
        ++reuseCount;
        return id;
    }

    // Allocate a new texture
    ++allocCount;
    return allocateGLMemory(desc);
}

void Texture2DPool::release(TextureID id) {
    MLN_TRACE_FUNC();

    assert(isUsed(id));
    auto desc = descriptions.at(id);
    auto set_it = pool.find(desc);
    set_it->second.used.erase(id);
    set_it->second.free.insert(id);
    lru_cache.touch(id);
    // Evict old textures if necessary
    evict();

    ++releaseCount;
}

TextureID Texture2DPool::allocateGLMemory(const Texture2DDesc& desc) {
    MLN_TRACE_FUNC();

    // Create handle
    TextureID id = 0;
    MBGL_CHECK_ERROR(glGenTextures(1, &id));
    assert(id != 0);

    // Allocate memory
    // Bind to TU 0 and upload
    context->activeTextureUnit = 0;
    context->texture[0] = id;
    MBGL_CHECK_ERROR(glTexImage2D(GL_TEXTURE_2D,
                                  0,
                                  Enum<gfx::TexturePixelType>::sizedFor(desc.pixelFormat, desc.channelType),
                                  desc.size.width,
                                  desc.size.height,
                                  0,
                                  Enum<gfx::TexturePixelType>::to(desc.pixelFormat),
                                  Enum<gfx::TextureChannelDataType>::to(desc.channelType),
                                  nullptr));

    // Update stats
    context->renderingStats().numCreatedTextures++;

    // Add to descriptions
    descriptions.emplace(id, desc);

    // Add to pool
    pool[desc].used.insert(id);

    // Update poolStorage
    auto storage = desc.getStorageSize();
    poolStorage += storage;
    context->renderingStats().memTextures += storage;
    MLN_TRACE_ALLOC_TEXTURE(id, storage);

    // Evict old textures if necessary
    evict();

    return id;
}

void Texture2DPool::freeAllocatedGLMemory(TextureID id) {
    MLN_TRACE_FUNC();

    // Remove from descriptions
    auto desc_it = descriptions.find(id);
    assert(desc_it != descriptions.end());
    auto desc = desc_it->second;
    descriptions.erase(desc_it);

    // Remove from descriptions lru_cache if it exists
    lru_cache.remove(id);

    // Remove from pool
    auto set_it = pool.find(desc);
    assert(set_it != pool.end());
    auto free_it = set_it->second.free.find(id);
    assert(free_it != set_it->second.free.end());
    set_it->second.free.erase(free_it);
    if (set_it->second.free.empty() && set_it->second.used.empty()) {
        pool.erase(set_it);
    }

    // Update poolStorage
    auto storage = desc.getStorageSize();
    poolStorage -= storage;
    context->renderingStats().memTextures -= storage;
    assert(context->renderingStats().memTextures >= 0);
    MLN_TRACE_FREE_TEXTURE(id);

    // Delete the texture
    MBGL_CHECK_ERROR(glDeleteTextures(1, &id));
    // Update stats
    context->renderingStats().numCreatedTextures--;

    ++freedCount;
}

void Texture2DPool::evict() {
    while (poolStorage > maxPoolStorage && !lru_cache.empty()) {
        TextureID id = lru_cache.evict();
        freeAllocatedGLMemory(id);
    }
}

void Texture2DPool::shrink() {
    MLN_TRACE_FUNC();

    while (!lru_cache.empty()) {
        TextureID id = lru_cache.evict();
        freeAllocatedGLMemory(id);
    }
}

bool Texture2DPool::isPooled(TextureID id) const {
    return descriptions.find(id) != descriptions.end();
}

bool Texture2DPool::isUsed(TextureID id) const {
    auto desc_it = descriptions.find(id);
    if (desc_it == descriptions.end()) {
        return false;
    }
    const auto& desc = desc_it->second;
    const auto& used_set = pool.at(desc).used;
    return used_set.find(id) != used_set.end();
}

bool Texture2DPool::isUnused(TextureID id) const {
    auto desc_it = descriptions.find(id);
    if (desc_it == descriptions.end()) {
        return false;
    }
    const auto& desc = desc_it->second;
    const auto& free_set = pool.at(desc).free;
    return free_set.find(id) != free_set.end();
}

size_t Texture2DPool::usedStorage() const {
    size_t total = 0;
    for (const auto& set : pool) {
        total += set.first.getStorageSize() * set.second.used.size();
    }
    return total;
}

size_t Texture2DPool::unusedStorage() const {
    size_t total = 0;
    for (const auto& set : pool) {
        total += set.first.getStorageSize() * set.second.free.size();
    }
    return total;
}

const Texture2DDesc& Texture2DPool::desc(TextureID id) const {
    assert(descriptions.find(id) != descriptions.end());
    return descriptions.at(id);
}

size_t Texture2DPool::storage(TextureID id) const {
    assert(descriptions.find(id) != descriptions.end());
    return descriptions.at(id).getStorageSize();
}

} // namespace gl
} // namespace mbgl
