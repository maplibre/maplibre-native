#pragma once

#include <mbgl/storage/response.hpp>
#include <mbgl/util/bitmask_operations.hpp>
#include <mbgl/util/font_stack.hpp>
#include <mbgl/util/tileset.hpp>

#include <string>
#include <optional>

namespace mbgl {

class Resource {
public:
    enum Kind : uint8_t {
        Unknown = 0,
        Style,
        Source,
        Tile,
        Glyphs,
        SpriteImage,
        SpriteJSON,
        Image
    };

    enum class Priority : bool {
        Regular,
        Low
    };

    enum class Usage : bool {
        Online,
        Offline
    };

    enum class StoragePolicy : bool {
        Permanent,
        Volatile
    };

    struct TileData {
        std::string urlTemplate;
        uint8_t pixelRatio;
        int32_t x;
        int32_t y;
        int8_t z;
    };

    enum class LoadingMethod : uint8_t {
        None = 0b00,
        Cache = 0b01,
        Network = 0b10,

        CacheOnly = Cache,
        NetworkOnly = Network,
        All = Cache | Network,
    };

    Resource(Kind kind_,
             std::string url_,
             std::optional<TileData> tileData_ = std::nullopt,
             LoadingMethod loadingMethod_ = LoadingMethod::All)
        : kind(kind_),
          loadingMethod(loadingMethod_),
          url(std::move(url_)),
          tileData(std::move(tileData_)) {}

    void setPriority(Priority p) { priority = p; }
    void setUsage(Usage u) { usage = u; }

    bool hasLoadingMethod(LoadingMethod method) const;

    static Resource style(const std::string& url);
    static Resource source(const std::string& url);
    static Resource tile(const std::string& urlTemplate,
                         float pixelRatio,
                         int32_t x,
                         int32_t y,
                         int8_t z,
                         Tileset::Scheme scheme,
                         LoadingMethod = LoadingMethod::All);
    static Resource glyphs(const std::string& urlTemplate,
                           const FontStack& fontStack,
                           const std::pair<uint16_t, uint16_t>& glyphRange);
    static Resource spriteImage(const std::string& base, float pixelRatio);
    static Resource spriteJSON(const std::string& base, float pixelRatio);
    static Resource image(const std::string& url);

    Kind kind;
    LoadingMethod loadingMethod;
    Usage usage{Usage::Online};
    Priority priority{Priority::Regular};
    std::string url;

    // Includes auxiliary data if this is a tile request.
    std::optional<TileData> tileData;

    std::optional<Timestamp> priorModified = std::nullopt;
    std::optional<Timestamp> priorExpires = std::nullopt;
    std::optional<std::string> priorEtag = std::nullopt;
    std::shared_ptr<const std::string> priorData;
    Duration minimumUpdateInterval{Duration::zero()};
    StoragePolicy storagePolicy{StoragePolicy::Permanent};
};

inline bool Resource::hasLoadingMethod(Resource::LoadingMethod method) const {
    return (loadingMethod & method);
}

} // namespace mbgl
