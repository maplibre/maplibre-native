#pragma once

#include <mbgl/style/image.hpp>
#include <mapbox/std/weak.hpp>
#include <mbgl/style/sprite.hpp>

#include <string>
#include <map>
#include <set>
#include <vector>
#include <array>
#include <memory>

namespace mbgl {

class FileSource;
class SpriteLoaderObserver;
class Scheduler;

class SpriteLoader {
public:
    SpriteLoader(float pixelRatio);
    ~SpriteLoader();

    void load(const std::unique_ptr<style::Sprite> sprite, FileSource&);

    void setObserver(SpriteLoaderObserver*);

private:
    void emitSpriteLoadedIfComplete(std::string id);

    // Invoked by SpriteAtlasWorker
    friend class SpriteLoaderWorker;

    const float pixelRatio;

    struct Data;
    std::unique_ptr<Data> data;

    SpriteLoaderObserver* observer = nullptr;
    std::shared_ptr<Scheduler> threadPool;
    mapbox::base::WeakPtrFactory<SpriteLoader> weakFactory{this};
};

} // namespace mbgl
