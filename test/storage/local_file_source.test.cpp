#include <mln/storage/local_file_source.hpp>
#include <mln/storage/resource.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/platform.hpp>
#include <mln/util/run_loop.hpp>
#include <mln/util/scoped.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <gtest/gtest.h>

namespace {

std::string fileURL(const std::filesystem::path& path) {
    const auto generic = path.generic_u8string();
    const std::string bytes(reinterpret_cast<const char*>(generic.data()), generic.size());

    constexpr char hex[] = "0123456789ABCDEF";
    std::string encoded;
    for (unsigned char byte : bytes) {
        if ((byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z') || (byte >= '0' && byte <= '9') ||
            byte == '-' || byte == '.' || byte == '_' || byte == '~' || byte == '/' || byte == ':') {
            encoded += static_cast<char>(byte);
        } else {
            encoded += '%';
            encoded += hex[byte >> 4];
            encoded += hex[byte & 0xF];
        }
    }

    return std::string(mln::util::FILE_PROTOCOL) + (encoded.starts_with('/') ? "" : "/") + encoded;
}

std::string toAbsoluteURL(const std::string& fileName) {
    return fileURL(std::filesystem::current_path() / "test" / "fixtures" / "storage" / "assets" / fileName);
}

} // namespace

using namespace mln;

TEST(LocalFileSource, AcceptsURL) {
    LocalFileSource fs(ResourceOptions::Default(), ClientOptions());
    EXPECT_TRUE(fs.canRequest(Resource::style("file://empty")));
    EXPECT_TRUE(fs.canRequest(Resource::style("file:///test")));
    EXPECT_FALSE(fs.canRequest(Resource::style("flie://foo")));
    EXPECT_FALSE(fs.canRequest(Resource::style("file:")));
    EXPECT_FALSE(fs.canRequest(Resource::style("style.json")));
    EXPECT_FALSE(fs.canRequest(Resource::style("")));
}

TEST(LocalFileSource, EmptyFile) {
    util::RunLoop loop;

    LocalFileSource fs(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = fs.request({Resource::Unknown, toAbsoluteURL("empty")}, [&](Response res) {
        req.reset();
        EXPECT_EQ(nullptr, res.error);
        ASSERT_TRUE(res.data.get());
        EXPECT_EQ("", *res.data);
        loop.stop();
    });

    loop.run();
}

TEST(LocalFileSource, NonEmptyFile) {
    util::RunLoop loop;

    LocalFileSource fs(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = fs.request({Resource::Unknown, toAbsoluteURL("nonempty")}, [&](Response res) {
        req.reset();
        EXPECT_EQ(nullptr, res.error);
        ASSERT_TRUE(res.data.get());
        EXPECT_EQ("content is here\n", *res.data);
        loop.stop();
    });

    loop.run();
}

TEST(LocalFileSource, NonASCIIPath) {
    const auto root = std::filesystem::temp_directory_path() / "mln-local-file-source-non-ascii";

    const Scoped cleanup([&] {
        std::error_code error;
        std::filesystem::remove_all(root, error);
    });

    std::error_code error;
    std::filesystem::remove_all(root, error);

    const auto directory = root / u8"ké 地図";
    ASSERT_TRUE(std::filesystem::create_directories(directory));

    const std::string body = "content is here\n";
    const auto file = directory / u8"スタイル.json";
    {
        std::ofstream out(file, std::ios::binary);
        ASSERT_TRUE(out.good());
        out << body;
    }

    util::RunLoop loop;

    LocalFileSource fs(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = fs.request(Resource::style(fileURL(file)), [&](Response res) {
        req.reset();
        EXPECT_EQ(nullptr, res.error);
        ASSERT_TRUE(res.data.get());
        EXPECT_EQ(body, *res.data);
        loop.stop();
    });

    loop.run();
}

#if defined(_WIN32)
TEST(LocalFileSource, InvalidUTF8Path) {
    util::RunLoop loop;
    LocalFileSource fs(ResourceOptions::Default(), ClientOptions());
    auto req = fs.request(Resource::style("file:///C:/%FF.json"), [&](Response res) {
        EXPECT_NE(nullptr, res.error);
        EXPECT_EQ(nullptr, res.data);
        loop.stop();
    });
    loop.run();
}
#endif

TEST(LocalFileSource, PartialFile) {
    util::RunLoop loop;

    LocalFileSource fs(ResourceOptions::Default(), ClientOptions());

    Resource resource(Resource::Unknown, toAbsoluteURL("nonempty"));
    resource.dataRange = std::make_pair<uint64_t, uint64_t>(4, 12);

    std::unique_ptr<AsyncRequest> req = fs.request(resource, [&](Response res) {
        req.reset();
        EXPECT_EQ(nullptr, res.error);
        ASSERT_TRUE(res.data.get());
        EXPECT_EQ("ent is he", *res.data);
        loop.stop();
    });

    loop.run();
}

TEST(LocalFileSource, NonExistentFile) {
    util::RunLoop loop;

    LocalFileSource fs(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = fs.request({Resource::Unknown, toAbsoluteURL("does_not_exist")},
                                                   [&](Response res) {
                                                       req.reset();
                                                       ASSERT_NE(nullptr, res.error);
                                                       EXPECT_EQ(Response::Error::Reason::NotFound, res.error->reason);
                                                       ASSERT_FALSE(res.data.get());
                                                       // Do not assert on platform-specific error message.
                                                       loop.stop();
                                                   });

    loop.run();
}

TEST(LocalFileSource, InvalidURL) {
    util::RunLoop loop;

    LocalFileSource fs(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = fs.request({Resource::Unknown, "test://wrong-scheme"}, [&](Response res) {
        req.reset();
        ASSERT_NE(nullptr, res.error);
        EXPECT_EQ(Response::Error::Reason::Other, res.error->reason);
        EXPECT_EQ("Invalid file URL", res.error->message);
        ASSERT_FALSE(res.data.get());
        loop.stop();
    });

    loop.run();
}

TEST(LocalFileSource, ReadDirectory) {
    util::RunLoop loop;

    LocalFileSource fs(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = fs.request({Resource::Unknown, toAbsoluteURL("directory")}, [&](Response res) {
        req.reset();
        ASSERT_NE(nullptr, res.error);
        EXPECT_EQ(Response::Error::Reason::NotFound, res.error->reason);
        ASSERT_FALSE(res.data.get());
        // Do not assert on platform-specific error message.
        loop.stop();
    });

    loop.run();
}

TEST(LocalFileSource, URLEncoding) {
    util::RunLoop loop;

    LocalFileSource fs(ResourceOptions::Default(), ClientOptions());

    const std::string url = toAbsoluteURL("") + "%6eonempty";

    std::unique_ptr<AsyncRequest> req = fs.request({Resource::Unknown, url}, [&](Response res) {
        req.reset();
        EXPECT_EQ(nullptr, res.error);
        ASSERT_TRUE(res.data.get());
        EXPECT_EQ("content is here\n", *res.data);
        loop.stop();
    });

    loop.run();
}

TEST(LocalFileSource, URLLimit) {
    util::RunLoop loop;

    size_t length = PATH_MAX - toAbsoluteURL("").size();
    LocalFileSource fs(ResourceOptions::Default(), ClientOptions());
    char* filename = new char[length];
    memset(filename, 'x', length);

    std::string url(filename, length);

    delete[] filename;

    std::unique_ptr<AsyncRequest> req = fs.request({Resource::Unknown, toAbsoluteURL(url)}, [&](Response res) {
        req.reset();
        ASSERT_NE(nullptr, res.error);
#if defined(WIN32)
        // MSYS2 and Microsoft Visual Studio defines PATH_MAX as 260, less than the
        // limit to trigger an error with reason "Other"
        EXPECT_EQ(Response::Error::Reason::NotFound, res.error->reason);
#else
        EXPECT_EQ(Response::Error::Reason::Other, res.error->reason);
#endif
        ASSERT_FALSE(res.data.get());
        loop.stop();
    });

    loop.run();
}
