#pragma once

#include <mln/util/tile_server_options.hpp>

#include <cstdint>
#include <memory>
#include <string>

namespace mln {

/**
 * @brief Holds values for resource options.
 */
class ResourceOptions final {
public:
    /**
     * @brief Constructs a ResourceOptions object with default values.
     */
    ResourceOptions();
    ~ResourceOptions();

    ResourceOptions(ResourceOptions&&) noexcept;
    ResourceOptions& operator=(const ResourceOptions& options);
    ResourceOptions& operator=(ResourceOptions&& options);

    ResourceOptions clone() const;

    /**
     * @brief Sets the Mapbox access token - see
     * https://docs.mapbox.com/help/how-mapbox-works/access-tokens/ for details.
     *
     * @param token Mapbox access token.
     * @return ResourceOptions for chaining options together.
     */
    ResourceOptions& withApiKey(std::string token);

    /**
     * @brief Gets the previously set (or default) Mapbox access token.
     *
     * @return const std::string& Mapbox access token.
     */
    const std::string& apiKey() const;

    /**
     * @brief Sets the tile server options..
     *
     * @param tileServerOptions Tile server options.
     * @return ResourceOptions for chaining options together.
     */
    ResourceOptions& withTileServerOptions(TileServerOptions tileServerOptions);

    /**
     * @brief Gets the previously set (or default) TileServerOptions.
     *
     * @return const TileServerOptions tile server options.
     */
    const TileServerOptions tileServerOptions() const;

    /**
     * @brief Sets the cache path.
     *
     * @param path Cache path.
     * @return ResourceOptions for chaining options together.
     */
    ResourceOptions& withCachePath(std::string path);

    /**
     * @brief Gets the previously set (or default) cache path.
     *
     * @return cache path
     */
    const std::string& cachePath() const;

    /**
     * @brief Sets the asset path, which is the root directory from where
     * the asset:// scheme gets resolved in a style.
     *
     * @param path Asset path.
     * @return ResourceOptions for chaining options together.
     */
    ResourceOptions& withAssetPath(std::string path);

    /**
     * @brief Gets the previously set (or default) asset path.
     *
     * @return asset path
     */
    const std::string& assetPath() const;

    /**
     * @brief Sets the maximum ambient cache size
     *
     * Applied when the database file source for these options is first opened.
     * That source is reused for the same tile server, API key, and cache path,
     * so a later change to this value does not resize an already-open database.
     * The default is 50 MB. A limit set on a file source opened with a different
     * tile server or API key does not apply to this one.
     *
     * @param size Cache maximum size in bytes. `0` disables the cache when the
     *             database has no offline regions.
     * @return reference to ResourceOptions for chaining options together.
     */
    ResourceOptions& withMaximumCacheSize(uint64_t size);

    /**
     * @brief Gets the previously set (or default) maximum allowed cache size.
     *
     * @return maximum allowed cache database size in bytes.
     */
    uint64_t maximumCacheSize() const;

    /**
     * @brief Sets the platform context. A platform context is usually an object
     * that assists the creation of a file source.
     *
     * @param context Platform context.
     * @return reference to ResourceOptions for chaining options together.
     */
    ResourceOptions& withPlatformContext(void* context);

    /**
     * @brief Gets the previously set (or default) platform context.
     *
     * @return Platform context.
     */
    void* platformContext() const;

    /**
     * @brief Returns default resource options.
     *
     * @return Resource options.
     */
    static ResourceOptions Default();

private:
    ResourceOptions(const ResourceOptions&);

    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace mln
