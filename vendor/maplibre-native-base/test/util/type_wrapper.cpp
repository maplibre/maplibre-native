#include "mapbox/util/type_wrapper.hpp"

#include <gtest/gtest.h>

using mapbox::base::TypeWrapper;

namespace {

class TestType {
public:
    TestType() { str[0] = 'a'; }

    // Detect moves
    TestType(TestType&& t) noexcept : i1(t.i1 + 1), i2(t.i2 + 2) { str[0] = t.str[0] + 1; }

    TestType(const TestType&) = delete;
    TestType& operator=(const TestType&) = delete;

    int i1 = 0;
    int i2 = 1;
    char str[256] = {};
};

} // namespace

TEST(TypeWrapper, Empty) {
    EXPECT_FALSE(TypeWrapper().has_value());
}

TEST(TypeWrapper, BasicTypes) {
    TypeWrapper i = 3;
    EXPECT_TRUE(i.has_value());
    EXPECT_EQ(i.get<decltype(3)>(), 3);

    auto iValue = i.get<decltype(3)>();
    EXPECT_EQ(iValue, 3);

    EXPECT_TRUE(TypeWrapper(4).has_value());
    EXPECT_EQ(TypeWrapper(4).get<decltype(4)>(), 4);

    TypeWrapper f = 6.2f;
    EXPECT_TRUE(f.has_value());
    EXPECT_EQ(f.get<decltype(6.2f)>(), 6.2f);

    const float fValue = f.get<decltype(6.2f)>();
    EXPECT_EQ(fValue, 6.2f);

    EXPECT_TRUE(TypeWrapper(1.0f).has_value());
    EXPECT_EQ(TypeWrapper(1.0f).get<decltype(1.0f)>(), 1.0f);

    TypeWrapper c = 'z';
    EXPECT_TRUE(c.has_value());
    EXPECT_EQ(c.get<decltype('z')>(), 'z');

    EXPECT_TRUE(TypeWrapper('z').has_value());
    EXPECT_EQ(TypeWrapper('z').get<decltype('z')>(), 'z');
}

TEST(TypeWrapper, TypesMove) {
    TypeWrapper i = 3;
    EXPECT_TRUE(i.has_value());

    TypeWrapper f = 6.2f;
    EXPECT_TRUE(f.has_value());

    f = std::move(i);
    EXPECT_FALSE(i.has_value()); // NOLINT(bugprone-use-after-move)

    EXPECT_TRUE(f.has_value());
    EXPECT_EQ(f.get<decltype(3)>(), 3);
}

TEST(TypeWrapper, SmallType) {
    struct T {
        explicit T(int32_t* p_) : p(p_) { (*p)++; }

        T(T&& t) noexcept : p(t.p) { (*p)++; }

        ~T() { (*p)--; }

        T(const T&) = delete;
        T& operator=(const T&) = delete;

        int32_t* p;
    };

    int32_t p = 0;

    {
        TypeWrapper u1 = TypeWrapper(T(&p));
        EXPECT_EQ(p, 1);

        auto u2(std::move(u1));
        EXPECT_EQ(p, 1);
    }

    EXPECT_EQ(p, 0);
}

TEST(TypeWrapper, LargeType) {
    TestType t1;
    TypeWrapper u1 = TypeWrapper(std::move(t1));
    EXPECT_TRUE(u1.has_value());

    // TestType should be moved into owning TypeWrapper
    EXPECT_EQ(u1.get<TestType>().i1, 1);

    auto u2(std::move(u1));
    EXPECT_FALSE(u1.has_value()); // NOLINT(bugprone-use-after-move)

    // TestType should not be moved when owning TypeWrapper is moved;
    EXPECT_EQ(u2.get<TestType>().i1, 1);

    // TestType should not be moved out of owning TypeWrapper
    auto& t2 = u2.get<TestType>();
    EXPECT_TRUE(u2.has_value());
    EXPECT_EQ(t2.i1, 1);
}

TEST(TypeWrapper, Pointer) {
    auto* t1 = new TestType(); // NOLINT cppcoreguidelines-owning-memory

    auto u1 = TypeWrapper(t1);
    EXPECT_TRUE(u1.has_value());

    // Only the pointer should be moved
    TestType* t2 = u1.get<TestType*>(); // NOLINT cppcoreguidelines-owning-memory
    EXPECT_EQ(t2->i1, 0);

    TypeWrapper u2(4);
    std::swap(u2, u1);

    EXPECT_TRUE(u1.has_value());
    EXPECT_TRUE(u2.has_value());

    t2 = u2.get<TestType*>();
    EXPECT_EQ(t2->i1, 0);
    delete t2; // NOLINT cppcoreguidelines-owning-memory
}

TEST(TypeWrapper, UniquePtr) {
    auto t1 = std::make_unique<TestType>();
    auto u1 = TypeWrapper(std::move(t1));

    EXPECT_EQ(t1, nullptr);
    EXPECT_TRUE(u1.has_value());

    u1 = TypeWrapper();
    EXPECT_FALSE(u1.has_value());

    TypeWrapper u2;
    auto* t3 = new TestType(); // NOLINT cppcoreguidelines-owning-memory
    u2 = std::unique_ptr<TestType>(t3);
    EXPECT_TRUE(u2.has_value());
}

TEST(TypeWrapper, SharedPtr) {
    std::shared_ptr<int> shared(new int(3));
    std::weak_ptr<int> weak = shared;
    TypeWrapper u1 = 0;

    EXPECT_EQ(weak.use_count(), 1);
    TypeWrapper u2 = shared;
    EXPECT_EQ(weak.use_count(), 2);

    u1 = std::move(u2);
    EXPECT_EQ(weak.use_count(), 2);
    std::swap(u2, u1);
    EXPECT_EQ(weak.use_count(), 2);
    u2 = 0;
    EXPECT_EQ(weak.use_count(), 1);
    shared = nullptr;
    EXPECT_EQ(weak.use_count(), 0);
}
