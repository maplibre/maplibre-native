#pragma once

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/style/image.hpp>
#include <mbgl/style/sprite.hpp>
#include <mapbox/std/weak.hpp>

#include <string>
#include <map>
#include <memory>

namespace mbgl {

class FileSource;
class SpriteLoaderObserver;

// NOTE: Any derived class must invalidate `weakFactory` in the destructor
class SpriteLoader final {
public:
    SpriteLoader(float pixelRatio, const TaggedScheduler& threadPool_);
    ~SpriteLoader();

    void load(const std::optional<style::Sprite> sprite, FileSource&);

    void setObserver(SpriteLoaderObserver*);

private:
    void emitSpriteLoadedIfComplete(style::Sprite sprite);

    // Invoked by SpriteAtlasWorker
    friend class SpriteLoaderWorker;

    const float pixelRatio;

    struct Data;
    std::map<std::string, std::unique_ptr<Data>> dataMap;
    std::mutex dataMapMutex;

    SpriteLoaderObserver* observer = nullptr;
    TaggedScheduler threadPool;
    mapbox::base::WeakPtrFactory<SpriteLoader> weakFactory{this};
    // Do not add members here, see `WeakPtrFactory`
};

} // namespace mbgl
