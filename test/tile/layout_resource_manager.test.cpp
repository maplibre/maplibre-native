#include <mln/test/stub_file_source.hpp>
#include <mln/tile/layout_resource_manager.hpp>
#include <mln/storage/response.hpp>
#include <mln/util/run_loop.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>

using namespace mln;

namespace {

class Requestor final : public LayoutResourceRequestor {
public:
    void onLayoutResourcesAvailable(LayoutResourceMap resources) override {
        ++calls;
        for (auto& [url, resource] : resources) received.insert_or_assign(url, std::move(resource));
    }

    unsigned calls = 0;
    LayoutResourceMap received;
};

struct Fixture {
    util::RunLoop loop;
    StubFileSource fileSource{StubFileSource::ResponseType::Manual};
    LayoutResourceManager manager;
    std::vector<std::string> requested;
    bool fail = false;

    Fixture() {
        fileSource.response = [this](const Resource& resource) -> std::optional<Response> {
            requested.push_back(resource.url);
            Response response;
            if (fail) {
                response.error = std::make_unique<Response::Error>(Response::Error::Reason::NotFound, "missing");
            } else {
                response.data = std::make_shared<std::string>("bytes of " + resource.url);
            }
            return response;
        };
    }
};

} // namespace

TEST(LayoutResourceManager, LoadsAUrlOnceForAllRequestorsThatWaitForIt) {
    Fixture fixture;
    Requestor first, second;
    fixture.manager.getResources(first, {"asset://car.glb"}, &fixture.fileSource);
    fixture.manager.getResources(second, {"asset://car.glb", "asset://truck.glb"}, &fixture.fileSource);
    EXPECT_EQ(0u, first.calls);
    EXPECT_EQ(0u, second.calls);

    fixture.fileSource.respondToAll();

    EXPECT_EQ((std::vector<std::string>{"asset://car.glb", "asset://truck.glb"}), [&] {
        auto urls = fixture.requested;
        std::sort(urls.begin(), urls.end());
        return urls;
    }());
    ASSERT_EQ(1u, first.received.size());
    ASSERT_EQ(2u, second.received.size());
    EXPECT_EQ(first.received.at("asset://car.glb"), second.received.at("asset://car.glb"));
    ASSERT_TRUE(second.received.at("asset://truck.glb")->data);
    EXPECT_EQ("bytes of asset://truck.glb", *second.received.at("asset://truck.glb")->data);
}

TEST(LayoutResourceManager, HandsOutALoadedResourceAtOnceWhileATileUsesIt) {
    Fixture fixture;
    Requestor first, second;
    fixture.manager.getResources(first, {"asset://car.glb"}, &fixture.fileSource);
    fixture.fileSource.respondToAll();

    fixture.manager.getResources(second, {"asset://car.glb"}, &fixture.fileSource);
    EXPECT_EQ(1u, second.calls);
    EXPECT_EQ(first.received.at("asset://car.glb"), second.received.at("asset://car.glb"));
    EXPECT_EQ(1u, fixture.requested.size());
}

TEST(LayoutResourceManager, RequestsAgainOnceNoTileUsesTheResource) {
    Fixture fixture;
    std::weak_ptr<const LayoutResource> released;
    {
        Requestor first;
        fixture.manager.getResources(first, {"asset://car.glb"}, &fixture.fileSource);
        fixture.fileSource.respondToAll();
        released = first.received.at("asset://car.glb");
        fixture.manager.removeRequestor(first);
    }
    EXPECT_TRUE(released.expired());

    Requestor second;
    fixture.manager.getResources(second, {"asset://car.glb"}, &fixture.fileSource);
    EXPECT_EQ(0u, second.calls);
    fixture.fileSource.respondToAll();
    EXPECT_EQ(1u, second.calls);
    EXPECT_EQ(2u, fixture.requested.size());
}

TEST(LayoutResourceManager, HandsOutAFailedLoadWithoutDataAndRetriesItForTheNextTile) {
    Fixture fixture;
    fixture.fail = true;
    Requestor first, second;
    fixture.manager.getResources(first, {"asset://car.glb"}, &fixture.fileSource);
    fixture.fileSource.respondToAll();
    ASSERT_EQ(1u, first.received.size());
    EXPECT_EQ(nullptr, first.received.at("asset://car.glb")->data);

    fixture.fail = false;
    fixture.manager.getResources(second, {"asset://car.glb"}, &fixture.fileSource);
    EXPECT_EQ(0u, second.calls);
    fixture.fileSource.respondToAll();
    ASSERT_TRUE(second.received.at("asset://car.glb")->data);
    EXPECT_EQ(2u, fixture.requested.size());
}

TEST(LayoutResourceManager, HandsOutResourcesWithoutDataWithoutAFileSource) {
    Fixture fixture;
    Requestor requestor;
    fixture.manager.getResources(requestor, {"asset://car.glb"}, nullptr);
    EXPECT_EQ(1u, requestor.calls);
    EXPECT_EQ(nullptr, requestor.received.at("asset://car.glb")->data);
}

TEST(LayoutResourceManager, StopsNotifyingARemovedRequestor) {
    Fixture fixture;
    Requestor removed, kept;
    fixture.manager.getResources(removed, {"asset://car.glb"}, &fixture.fileSource);
    fixture.manager.getResources(kept, {"asset://car.glb"}, &fixture.fileSource);
    fixture.manager.removeRequestor(removed);
    fixture.fileSource.respondToAll();
    EXPECT_EQ(0u, removed.calls);
    EXPECT_EQ(1u, kept.calls);
}

TEST(LayoutResourceManager, CancelsARequestNoTileWaitsFor) {
    Fixture fixture;
    Requestor requestor;
    fixture.manager.getResources(requestor, {"asset://car.glb"}, &fixture.fileSource);
    fixture.manager.removeRequestor(requestor);
    fixture.fileSource.respondToAll();
    EXPECT_TRUE(fixture.requested.empty());
}

TEST(LayoutResource, CreatesAnObjectOncePerOwnerAcrossThreads) {
    const LayoutResource resource(std::make_shared<const std::string>("bytes"));
    std::atomic<int> creates{0};
    const auto create = [&](const std::string& data) -> std::shared_ptr<void> {
        ++creates;
        return std::make_shared<std::string>(data);
    };
    const int owner = 0, otherOwner = 0;
    std::vector<std::shared_ptr<void>> made(8);
    std::vector<std::thread> threads;
    for (auto& result : made) threads.emplace_back([&] { result = resource.getOrCreate(&owner, create); });
    for (auto& thread : threads) thread.join();
    EXPECT_EQ(1, creates.load());
    for (const auto& result : made) EXPECT_EQ(made.front(), result);

    EXPECT_NE(made.front(), resource.getOrCreate(&otherOwner, create));
    EXPECT_EQ(2, creates.load());
}

TEST(LayoutResource, CreatesNothingFromAFailedLoad) {
    const LayoutResource resource(nullptr);
    const int owner = 0;
    bool called = false;
    EXPECT_EQ(nullptr, resource.getOrCreate(&owner, [&](const std::string&) -> std::shared_ptr<void> {
        called = true;
        return std::make_shared<int>(1);
    }));
    EXPECT_FALSE(called);
}
