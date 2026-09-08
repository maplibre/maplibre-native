#include <mln/layermanager/layer_manager.hpp>
#include <mln/layermanager/layer_factory.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/style/rapidjson_conversion.hpp>

#include <gtest/gtest.h>
#include <atomic>
#include <thread>

using namespace mln;

namespace {
class Factory final : public LayerFactory {
public:
    Factory(std::string name_, std::atomic<int>& destroyed_, std::atomic<int>& created_)
        : name(std::move(name_)),
          destroyed(destroyed_),
          created(created_),
          info{name.c_str(),
               style::LayerTypeInfo::Source::NotRequired,
               style::LayerTypeInfo::Pass3D::NotRequired,
               style::LayerTypeInfo::Layout::NotRequired,
               style::LayerTypeInfo::FadingTiles::NotRequired,
               style::LayerTypeInfo::CrossTileIndex::NotRequired,
               style::LayerTypeInfo::TileKind::NotRequired} {}
    ~Factory() override { ++destroyed; }
    const style::LayerTypeInfo* getTypeInfo() const noexcept override { return &info; }
    std::unique_ptr<style::Layer> createLayer(const std::string&,
                                              const style::conversion::Convertible&) noexcept override {
        ++created;
        return nullptr;
    }
    std::unique_ptr<RenderLayer> createRenderLayer(Immutable<style::Layer::Impl>) noexcept override { return nullptr; }

private:
    const std::string name;
    std::atomic<int>& destroyed;
    std::atomic<int>& created;
    const style::LayerTypeInfo info;
};

// An isolated manager keeps tests independent of process-wide registration.
class Manager final : public LayerManager {
public:
    ~Manager() override = default;

private:
    LayerFactory* getFactory(const std::string&) noexcept override { return nullptr; }
    LayerFactory* getFactory(const style::LayerTypeInfo*) noexcept override { return nullptr; }
};
} // namespace

TEST(LayerManager, OwnsFactoriesAndDispatchesOutsideRegistrationLock) {
    std::atomic<int> destroyed{0}, created{0};
    {
        Manager manager;
        std::string error;
        ASSERT_TRUE(manager.registerLayerFactory(std::make_unique<Factory>("test-owned", destroyed, created), error));
        EXPECT_TRUE(error.empty());
        EXPECT_TRUE(manager.hasLayerType("test-owned"));
        JSDocument json;
        json.SetObject();
        style::conversion::Error parseError;
        const JSValue* value = &json;
        EXPECT_EQ(nullptr,
                  manager.createLayer("test-owned", "test", style::conversion::Convertible(value), parseError));
        EXPECT_EQ(1, created);
        EXPECT_EQ(0, destroyed);
        EXPECT_FALSE(manager.hasLayerType("unknown"));
    }
    EXPECT_EQ(1, destroyed);
}

TEST(LayerManager, RejectsMalformedFactoriesAndAtomicBatchConflicts) {
    std::atomic<int> destroyed{0}, created{0};
    Manager manager;
    std::string error;
    EXPECT_FALSE(manager.registerLayerFactory(nullptr, error));
    EXPECT_FALSE(manager.registerLayerFactory(std::make_unique<Factory>("", destroyed, created), error));
    ASSERT_TRUE(manager.registerLayerFactory(std::make_unique<Factory>("occupied", destroyed, created), error));
    std::vector<std::unique_ptr<LayerFactory>> batch;
    batch.push_back(std::make_unique<Factory>("new", destroyed, created));
    batch.push_back(std::make_unique<Factory>("occupied", destroyed, created));
    EXPECT_FALSE(manager.registerLayerFactories(std::move(batch), error));
    EXPECT_FALSE(manager.hasLayerType("new"));
    EXPECT_NE(std::string::npos, error.find("occupied"));
    batch.push_back(std::make_unique<Factory>("duplicate", destroyed, created));
    batch.push_back(std::make_unique<Factory>("duplicate", destroyed, created));
    EXPECT_FALSE(manager.registerLayerFactories(std::move(batch), error));
    EXPECT_FALSE(manager.hasLayerType("duplicate"));
}

TEST(LayerManager, RejectsBuiltInNames) {
    std::atomic<int> destroyed{0}, created{0};
    std::string error;
    ASSERT_TRUE(LayerManager::get()->hasLayerType("circle"));
    EXPECT_FALSE(
        LayerManager::get()->registerLayerFactory(std::make_unique<Factory>("circle", destroyed, created), error));
    EXPECT_EQ(1, destroyed);
}

TEST(LayerManager, ConcurrentRegistrationHasOneWinner) {
    std::atomic<int> destroyed{0}, created{0}, winners{0};
    Manager manager;
    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back([&] {
            std::string error;
            if (manager.registerLayerFactory(std::make_unique<Factory>("raced", destroyed, created), error)) ++winners;
            EXPECT_TRUE(manager.hasLayerType("raced"));
        });
    }
    for (auto& thread : threads) thread.join();
    EXPECT_EQ(1, winners);
    EXPECT_EQ(7, destroyed);
}
