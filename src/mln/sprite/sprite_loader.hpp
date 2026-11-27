#pragma once

#include <mln/actor/scheduler.hpp>
#include <mln/style/image.hpp>
#include <mln/style/sprite.hpp>
#include <mln/util/callback_scope.hpp>

#include <string>
#include <map>
#include <memory>
#include <mutex>

namespace mln {

class FileSource;
class SpriteLoaderObserver;

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
    util::CallbackScope callbacks;
};

} // namespace mln
