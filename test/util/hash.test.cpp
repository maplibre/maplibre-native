#include <mbgl/test/util.hpp>

#include <mbgl/util/hash.hpp>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <random>
#include <vector>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/shaders/mtl/background.hpp>
#include <mbgl/shaders/mtl/circle.hpp>
#include <mbgl/shaders/mtl/clipping_mask.hpp>
#include <mbgl/shaders/mtl/collision.hpp>
#include <mbgl/shaders/mtl/custom_geometry.hpp>
#include <mbgl/shaders/mtl/custom_symbol_icon.hpp>
#include <mbgl/shaders/mtl/debug.hpp>
#include <mbgl/shaders/mtl/fill.hpp>
#include <mbgl/shaders/mtl/fill_extrusion.hpp>
#include <mbgl/shaders/mtl/heatmap.hpp>
#include <mbgl/shaders/mtl/heatmap_texture.hpp>
#include <mbgl/shaders/mtl/hillshade.hpp>
#include <mbgl/shaders/mtl/hillshade_prepare.hpp>
#include <mbgl/shaders/mtl/line.hpp>
#include <mbgl/shaders/mtl/location_indicator.hpp>
#include <mbgl/shaders/mtl/fill.hpp>
#include <mbgl/shaders/mtl/raster.hpp>
#include <mbgl/shaders/mtl/symbol.hpp>
#endif

using namespace mbgl;

TEST(OrderIndependentHash, Permutations) {
    // Try collections of up to this many elements
    constexpr int maxSize = 6;
    // Try this many different sets of values for each size
    constexpr int iterations = 10;
    // Use values up to this large
    constexpr size_t maxVal = 10;
    // Repeat all that with different random seeds
    constexpr int seeds = 5;

    for (int curSeed = 0; curSeed < seeds; ++curSeed) {
        std::seed_seq seed{0xf00dLL + curSeed};
        std::default_random_engine engine(seed);
        std::uniform_int_distribution<size_t> distribution(0, maxVal);

        for (int curSize = 1; curSize <= maxSize; ++curSize) {
            for (int ii = 0; ii < iterations; ++ii) {
                // Generate `curSize` random values
                std::vector<size_t> values;
                for (int jj = 0; jj < curSize; ++jj) {
                    values.push_back(distribution(engine));
                }

                // Generate the hash for each permutation, comparing subsequent values to the first one.
                bool isFirst = true;
                size_t initialValue = 0;
                do {
                    const auto curValue = util::order_independent_hash(values.begin(), values.end());
                    if (isFirst) {
                        initialValue = curValue;
                        isFirst = false;
                    } else {
                        EXPECT_EQ(initialValue, curValue);
                    }
                } while (std::next_permutation(values.begin(), values.end()));
            }
        }
    }
}

#if MLN_RENDER_BACKEND_METAL

using namespace mbgl::shaders;

template <typename TSet, typename TIter = typename TSet::const_iterator, typename TFunc>
void each_subset(const TSet& sofar, TIter beg, TIter end, TFunc f) {
    if (beg == end) {
        f(sofar.cbegin(), sofar.cend());
    } else {
        TSet with = sofar;
        with.insert(*beg);
        each_subset<TSet, TIter>(with, std::next(beg), end, f);
        each_subset<TSet, TIter>(sofar, std::next(beg), end, f);
    }
}

/// Ensure that no combination of attributes used by a shader definition produce a hash conflict
template <typename ShaderType>
void checkShaderHashes() {
    using AttribSet = mbgl::unordered_set<size_t>;
    AttribSet attributes;
    for (const auto& attrib : ShaderType::attributes) {
        attributes.insert(attrib.id);
    }

    std::set<std::size_t> subsetHashes;
    each_subset(AttribSet{}, attributes.cbegin(), attributes.cend(), [&](auto beg, auto end) {
        if (beg != end) {
            std::vector<size_t> subset(beg, end);
            std::optional<std::size_t> firstHash;
            do {
                const auto hash = util::order_independent_hash(subset.begin(), subset.end());
                if (firstHash) {
                    // All permutations of the subset must yield the same hash key as the first
                    EXPECT_EQ(*firstHash, hash);
                } else {
                    firstHash.emplace(hash);
                    // All subset hashes must be unique
                    EXPECT_TRUE(subsetHashes.insert(hash).second);
                }
            } while (std::next_permutation(subset.begin(), subset.end()));
        }
    });
    EXPECT_EQ(static_cast<size_t>(std::pow(2, attributes.size())), 1 + subsetHashes.size());
}

template <BuiltIn... type>
void checkShaderHashes() {
    // For OpenGL, we would need to do reflection
    //(checkShaderHashes<ShaderSource<type, gfx::Backend::Type::OpenGL>>(),...);
    (checkShaderHashes<ShaderSource<type, gfx::Backend::Type::Metal>>(), ...);
}

TEST(OrderIndependentHash, Shaders) {
    checkShaderHashes<BuiltIn::BackgroundShader,
                      BuiltIn::BackgroundPatternShader,
                      BuiltIn::CircleShader,
                      BuiltIn::CollisionBoxShader,
                      BuiltIn::CollisionCircleShader,
                      BuiltIn::CustomGeometryShader,
                      BuiltIn::CustomSymbolIconShader,
                      BuiltIn::DebugShader,
                      BuiltIn::FillShader,
                      BuiltIn::FillOutlineShader,
                      BuiltIn::FillPatternShader,
                      BuiltIn::FillOutlinePatternShader,
                      BuiltIn::FillOutlineTriangulatedShader,
                      BuiltIn::FillExtrusionShader,
                      BuiltIn::FillExtrusionPatternShader,
                      BuiltIn::HeatmapShader,
                      BuiltIn::HeatmapTextureShader,
                      BuiltIn::HillshadePrepareShader,
                      BuiltIn::HillshadeShader,
                      BuiltIn::LineShader,
                      BuiltIn::LineGradientShader,
                      BuiltIn::LinePatternShader,
                      BuiltIn::LineSDFShader,
                      BuiltIn::LocationIndicatorShader,
                      BuiltIn::LocationIndicatorTexturedShader,
                      BuiltIn::RasterShader,
                      BuiltIn::SymbolIconShader,
                      BuiltIn::SymbolSDFShader,
                      BuiltIn::SymbolTextAndIconShader>();
}

#endif
