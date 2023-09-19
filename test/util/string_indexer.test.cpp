#include <mbgl/test/util.hpp>

#include <mbgl/util/string_indexer.hpp>

#include <cstdint>

using namespace mbgl;

TEST(StringIndexer, AddStrings) {
    StringIndexer::clear();
    EXPECT_EQ(StringIndexer::size(), 0);

    const auto id1 = StringIndexer::get("test string1");
    EXPECT_EQ(id1, 0);
    EXPECT_EQ(StringIndexer::size(), 1);

    const std::string_view s2 = "test string2";
    const auto id2 = StringIndexer::get(s2);
    EXPECT_EQ(id2, 1);
    EXPECT_EQ(StringIndexer::size(), 2);
}

TEST(StringIndexer, GetString) {
    StringIndexer::clear();
    EXPECT_EQ(StringIndexer::size(), 0);

    constexpr auto str = "test string1";

    const auto id1 = StringIndexer::get(str);
    EXPECT_EQ(id1, 0);
    EXPECT_EQ(StringIndexer::size(), 1);

    const auto str1 = StringIndexer::get(id1);
    EXPECT_EQ(str, str1);

    const auto str2 = str;
    const auto id2 = StringIndexer::get(str2);
    EXPECT_EQ(id1, id2);
}

TEST(StringIndexer, GetOOBIdentity) {
    StringIndexer::clear();
    EXPECT_EQ(StringIndexer::size(), 0);

    std::string str;
    EXPECT_DEATH_IF_SUPPORTED(str = StringIndexer::get(42), "id < identityToString.size()");
    EXPECT_TRUE(str.empty());
    EXPECT_DEATH_IF_SUPPORTED(str = StringIndexer::get(-1), "id < identityToString.size()");
    EXPECT_TRUE(str.empty());
}
