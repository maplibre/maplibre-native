#include <mbgl/actor/actor.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <mbgl/sprite/sprite_loader.hpp>
#include <mbgl/sprite/sprite_loader_observer.hpp>
#include <mbgl/sprite/sprite_parser.hpp>
#include <mbgl/storage/file_source.hpp>
#include <mbgl/storage/resource.hpp>
#include <mbgl/storage/response.hpp>
#include <mbgl/util/async_request.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/exception.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/platform.hpp>
#include <mbgl/util/std.hpp>

#include <cassert>

namespace mbgl {

static SpriteLoaderObserver nullObserver;

struct SpriteLoader::Data {
    std::shared_ptr<const std::string> image;
    std::shared_ptr<const std::string> json;
    std::unique_ptr<AsyncRequest> jsonRequest;
    std::unique_ptr<AsyncRequest> spriteRequest;
};

SpriteLoader::SpriteLoader(float pixelRatio_)
    : pixelRatio(pixelRatio_),
      observer(&nullObserver),
      threadPool(Scheduler::GetBackground()) {}

SpriteLoader::~SpriteLoader() = default;

void SpriteLoader::load(const style::Sprite* sprite, FileSource& fileSource) {
    if (sprite == nullptr) {
        // Treat a non-existent sprite as a successfully loaded empty sprite.
        observer->onSpriteLoaded({});
        return;
    }
    
    std::string id = sprite->id;
    std::string url = sprite->spriteURL;
    
    dataMap[id] = std::make_unique<Data>();
    dataMap[id]->jsonRequest = fileSource.request(Resource::spriteJSON(url, pixelRatio), [this, id](Response res) {
        std::lock_guard<std::mutex> lock(dataMapMutex);
        Data* data = dataMap[id].get();
        if (res.error) {
            observer->onSpriteError(std::make_exception_ptr(std::runtime_error(res.error->message)));
        } else if (res.notModified) {
            return;
        } else if (res.noContent) {
            data->json = std::make_shared<std::string>();
            emitSpriteLoadedIfComplete(id);
        } else {
            // Only trigger a sprite loaded event we got new data.
            assert(data->json != res.data);
            data->json = std::move(res.data);
            emitSpriteLoadedIfComplete(id);
        }
    });

    dataMap[id]->spriteRequest = fileSource.request(Resource::spriteImage(url, pixelRatio), [this, id](Response res) {
        std::lock_guard<std::mutex> lock(dataMapMutex);
        Data* data = dataMap[id].get();
        if (res.error) {
            observer->onSpriteError(std::make_exception_ptr(std::runtime_error(res.error->message)));
        } else if (res.notModified) {
            return;
        } else if (res.noContent) {
            data->image = std::make_shared<std::string>();
            emitSpriteLoadedIfComplete(id);
        } else {
            assert(dataMap[id]->image != res.data);
            data->image = std::move(res.data);
            emitSpriteLoadedIfComplete(id);
        }
    });
}

void SpriteLoader::emitSpriteLoadedIfComplete(std::string id) {
    
    Data* data = dataMap[id].get();
    assert(data);
    if (!data->image || !data->json) {
        return;
    }

    struct ParseResult {
        std::vector<Immutable<style::Image::Impl>> images;
        std::exception_ptr error;
    };

    auto parseClosure = [id = id, image = data->image, json = data->json]() -> ParseResult {
        try {
            return {parseSprite(id, *image, *json), nullptr};
        } catch (...) {
            return {{}, std::current_exception()};
        }
    };

    auto resultClosure = [this, weak = weakFactory.makeWeakPtr()](ParseResult result) {
        if (!weak) return; // This instance has been deleted.

        if (result.error) {
            observer->onSpriteError(result.error);
            return;
        }
        observer->onSpriteLoaded(std::move(result.images));
    };

    threadPool->scheduleAndReplyValue(parseClosure, resultClosure);
}

void SpriteLoader::setObserver(SpriteLoaderObserver* observer_) {
    observer = observer_;
}

} // namespace mbgl
