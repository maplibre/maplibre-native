#include <mln/test/stub_file_source.hpp>
#include <mln/tile/layout_resource_manager.hpp>
#include <mln/storage/response.hpp>
#include <mln/util/run_loop.hpp>

#include <gtest/gtest.h>

#include <functional>

using namespace mln;

namespace {

class Requestor : public LayoutResourceRequestor {
public:
    explicit Requestor(std::function<void()> onUpdate_ = [] {})
        : onUpdate(std::move(onUpdate_)) {}

    void onLayoutResourcesAvailable(LayoutResourceMap resources) override {
        ++calls;
        for (auto& [url, resource] : resources) received.insert_or_assign(url, std::move(resource));
        onUpdate();
    }

    LayoutResourceMap received;
    int calls = 0;

private:
    std::function<void()> onUpdate;
};

Response dataResponse(const Resource& resource) {
    Response response;
    response.data = std::make_shared<std::string>("bytes of " + resource.url);
    return response;
}

} // namespace

TEST(LayoutResourceManager, LoadsEachURLOnceAndSharesIt) {
    util::RunLoop loop;
    StubFileSource fileSource;
    int requests = 0;
    fileSource.response = [&](const Resource& resource) -> std::optional<Response> {
        ++requests;
        return dataResponse(resource);
    };
    LayoutResourceManager manager;
    Requestor first([&] {
        if (first.received.size() == 2) loop.stop();
    });
    Requestor second;
    manager.getResources(first, {"test://a", "test://b"}, &fileSource);
    manager.getResources(second, {"test://a"}, &fileSource);
    EXPECT_TRUE(first.received.empty());
    loop.run();

    EXPECT_EQ(2, requests);
    ASSERT_EQ(1u, second.received.size());
    EXPECT_EQ(first.received.at("test://a"), second.received.at("test://a"));
    EXPECT_EQ("bytes of test://b", *first.received.at("test://b")->data);

    // A loaded resource is handed out at once, without another request.
    Requestor third;
    manager.getResources(third, {"test://b"}, &fileSource);
    EXPECT_EQ(1, third.calls);
    EXPECT_EQ(first.received.at("test://b"), third.received.at("test://b"));
    EXPECT_EQ(2, requests);
}

TEST(LayoutResourceManager, RetriesAFailedURLForTheNextRequestor) {
    StubFileSource fileSource(StubFileSource::ResponseType::Synchronous);
    bool fail = true;
    fileSource.response = [&](const Resource& resource) -> std::optional<Response> {
        if (!fail) return dataResponse(resource);
        Response response;
        response.error = std::make_unique<Response::Error>(Response::Error::Reason::NotFound, "missing");
        return response;
    };
    LayoutResourceManager manager;
    Requestor first;
    manager.getResources(first, {"test://a"}, &fileSource);
    ASSERT_EQ(1u, first.received.size());
    EXPECT_FALSE(first.received.at("test://a")->data);

    fail = false;
    Requestor second;
    manager.getResources(second, {"test://a"}, &fileSource);
    ASSERT_EQ(1u, second.received.size());
    EXPECT_EQ("bytes of test://a", *second.received.at("test://a")->data);
}

TEST(LayoutResourceManager, ForgetsRemovedRequestors) {
    util::RunLoop loop;
    StubFileSource fileSource;
    fileSource.response = [&](const Resource& resource) -> std::optional<Response> {
        return dataResponse(resource);
    };
    LayoutResourceManager manager;
    Requestor removed;
    Requestor kept([&] { loop.stop(); });
    manager.getResources(removed, {"test://a"}, &fileSource);
    manager.getResources(kept, {"test://a"}, &fileSource);
    manager.removeRequestor(removed);
    loop.run();
    EXPECT_EQ(0, removed.calls);
    EXPECT_EQ(1, kept.calls);
}

TEST(LayoutResourceManager, FailsAtOnceWithoutAFileSource) {
    LayoutResourceManager manager;
    Requestor requestor;
    manager.getResources(requestor, {"test://a"}, nullptr);
    ASSERT_EQ(1u, requestor.received.size());
    EXPECT_FALSE(requestor.received.at("test://a")->data);
}

TEST(LayoutResource, CreatesOncePerOwner) {
    const LayoutResource resource(std::make_shared<std::string>("bytes"));
    int creates = 0;
    const auto create = [&](const std::string& bytes) -> std::shared_ptr<void> {
        ++creates;
        return std::make_shared<std::string>(bytes);
    };
    const int firstOwner = 0, secondOwner = 0;
    const auto made = resource.getOrCreate(&firstOwner, create);
    EXPECT_EQ(made, resource.getOrCreate(&firstOwner, create));
    EXPECT_NE(made, resource.getOrCreate(&secondOwner, create));
    EXPECT_EQ(2, creates);

    // A resource without data never creates anything.
    const LayoutResource failed(nullptr);
    EXPECT_FALSE(failed.getOrCreate(&firstOwner, create));
    EXPECT_EQ(2, creates);
}
