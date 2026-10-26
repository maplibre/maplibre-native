#include <mln/shaders/feature_index.hpp>
#include <mln/test/util.hpp>

#include <array>
#include <string>
#include <unordered_set>
#include <vector>

using namespace mln;
using namespace mln::shaders;

namespace {

constexpr std::array<FeatureAttribute, 3> threeFields{{
    FeatureAttribute{1, "color", 4},
    FeatureAttribute{2, "opacity", 2},
    FeatureAttribute{3, "width", 2},
}};

const std::vector<std::array<float, 4>>* tableFor(std::size_t id,
                                                  const std::vector<std::array<float, 4>>& color,
                                                  const std::vector<std::array<float, 4>>& opacity,
                                                  const std::vector<std::array<float, 4>>& width) {
    if (id == 1) {
        return &color;
    }
    if (id == 2) {
        return &opacity;
    }
    if (id == 3) {
        return &width;
    }
    return nullptr;
}

} // namespace

TEST(FeatureTable, SkipsUniformField) {
    const std::unordered_set<std::size_t> uniforms{2};
    const auto layout = featureLayout(threeFields, uniforms);

    ASSERT_EQ(layout.fields.size(), 2u);
    EXPECT_STREQ(layout.fields[0].name, "color");
    EXPECT_EQ(layout.fields[0].offset, 0u);
    EXPECT_EQ(layout.fields[0].floatCount, 4u);
    EXPECT_STREQ(layout.fields[1].name, "width");
    EXPECT_EQ(layout.fields[1].offset, 4u);
    EXPECT_EQ(layout.fields[1].floatCount, 2u);
    EXPECT_EQ(layout.stride, 6u);

    const std::vector<std::array<float, 4>> color{{
        {1.f, 2.f, 3.f, 4.f},
        {5.f, 6.f, 7.f, 8.f},
    }};
    const std::vector<std::array<float, 4>> opacity{{{0.f, 1.f, 0.f, 0.f}}};
    const std::vector<std::array<float, 4>> width{{{9.f, 10.f, 0.f, 0.f}}};

    const auto packed = packFeatureTable(
        layout, 2, [&](std::size_t id) { return tableFor(id, color, opacity, width); });

    const std::vector<float> expected{1.f, 2.f, 3.f, 4.f, 9.f, 10.f, 5.f, 6.f, 7.f, 8.f, 0.f, 0.f};
    EXPECT_EQ(packed, expected);

    const auto source = featureLayoutSource(layout);
    EXPECT_NE(source.find("const MLN_FEATURE_STRIDE: u32 = 6u;"), std::string::npos);
    EXPECT_NE(source.find("const MLN_FEATURE_color: u32 = 0u;"), std::string::npos);
    EXPECT_NE(source.find("const MLN_FEATURE_width: u32 = 4u;"), std::string::npos);
    EXPECT_EQ(source.find("MLN_FEATURE_opacity"), std::string::npos);
}

TEST(FeatureTable, PacksWideFieldBeforeScalar) {
    constexpr std::array<FeatureAttribute, 2> catalog{{
        FeatureAttribute{10, "color", 4},
        FeatureAttribute{11, "width", 2},
    }};
    const std::unordered_set<std::size_t> uniforms;
    const auto layout = featureLayout(catalog, uniforms);

    ASSERT_EQ(layout.fields.size(), 2u);
    EXPECT_EQ(layout.fields[0].offset, 0u);
    EXPECT_EQ(layout.fields[1].offset, 4u);
    EXPECT_EQ(layout.stride, 6u);

    const std::vector<std::array<float, 4>> color{{{0.25f, 0.5f, 0.75f, 1.f}}};
    const std::vector<std::array<float, 4>> width{{{3.f, 4.f, 0.f, 0.f}}};

    const auto packed = packFeatureTable(layout, 1, [&](std::size_t id) {
        if (id == 10) {
            return &color;
        }
        if (id == 11) {
            return &width;
        }
        return static_cast<const std::vector<std::array<float, 4>>*>(nullptr);
    });

    const std::vector<float> expected{0.25f, 0.5f, 0.75f, 1.f, 3.f, 4.f};
    EXPECT_EQ(packed, expected);
}
