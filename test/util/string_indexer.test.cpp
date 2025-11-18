#include <mbgl/test/util.hpp>

#include <mbgl/util/string_indexer.hpp>

#include <cstdint>

using mbgl::stringIndexer;

// Allow public default construction
class StringIndexer : public mbgl::StringIndexer {
public:
    StringIndexer() {}
};

TEST(StringIndexer, SingletonStringIndexer) {
    EXPECT_GE(stringIndexer().size(), 0);
}

TEST(StringIndexer, AddStrings) {
    StringIndexer strIndexer;
    EXPECT_EQ(strIndexer.size(), 0);

    const auto id1 = strIndexer.get("test string1");
    EXPECT_EQ(id1, 0);
    EXPECT_EQ(strIndexer.size(), 1);

    const std::string_view s2 = "test string2";
    const auto id2 = strIndexer.get(s2);
    EXPECT_EQ(id2, 1);
    EXPECT_EQ(strIndexer.size(), 2);
}

TEST(StringIndexer, GetString) {
    StringIndexer strIndexer;
    EXPECT_EQ(strIndexer.size(), 0);

    constexpr auto str = "test string1";

    const auto id1 = strIndexer.get(str);
    EXPECT_EQ(id1, 0);
    EXPECT_EQ(strIndexer.size(), 1);

    const auto str1 = strIndexer.get(id1);
    EXPECT_EQ(str, str1);

    const auto str2 = str;
    const auto id2 = strIndexer.get(str2);
    EXPECT_EQ(id1, id2);
}

TEST(StringIndexer, Reallocate) {
    StringIndexer strIndexer;
    EXPECT_EQ(strIndexer.size(), 0);

    constexpr auto str = "reallocate test string1";

    const auto id1 = strIndexer.get(str);
    EXPECT_EQ(id1, 0);
    EXPECT_EQ(strIndexer.size(), 1);

    // force reallocation. assume capacity: 100 strings, 100 * 32 bytes buffer space
    constexpr auto N = 1000;
    auto string_for_i = [](int i) -> std::string {
        using namespace std::string_literals;
        return "1234567890"s + "1234567890"s + "1234567890"s + "---" + std::to_string(i);
    };
    for (auto i = 1; i < N; ++i) {
        strIndexer.get(string_for_i(i));
    }

    EXPECT_EQ(strIndexer.size(), N);

    const auto str1 = strIndexer.get(id1);
    EXPECT_EQ(str, str1);

    for (auto i = 1; i < N; ++i) {
        const auto strN1 = strIndexer.get(i);
        EXPECT_EQ(strN1, string_for_i(i));
    }
}

TEST(StringIndexer, GetOOBIdentity) {
    StringIndexer strIndexer;
    EXPECT_EQ(strIndexer.size(), 0);

    std::string str;
#ifndef NDEBUG
    EXPECT_DEATH_IF_SUPPORTED(str = strIndexer.get(42), "id < identityToString.size\\(\\)");
#endif
    EXPECT_TRUE(str.empty());
#ifndef NDEBUG
    EXPECT_DEATH_IF_SUPPORTED(str = strIndexer.get(-1), "id < identityToString.size\\(\\)");
#endif
    EXPECT_TRUE(str.empty());
}
