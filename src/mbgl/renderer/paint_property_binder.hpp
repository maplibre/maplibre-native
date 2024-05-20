#pragma once

#include <mbgl/gfx/attribute.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/uniform.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/layout/pattern_layout.hpp>
#include <mbgl/programs/attributes.hpp>
#include <mbgl/renderer/cross_faded_property_evaluator.hpp>
#include <mbgl/renderer/image_atlas.hpp>
#include <mbgl/renderer/paint_property_statistics.hpp>
#include <mbgl/renderer/possibly_evaluated_property_value.hpp>
#include <mbgl/util/indexed_tuple.hpp>
#include <mbgl/util/literal.hpp>
#include <mbgl/util/type_list.hpp>
#include <mbgl/util/variant.hpp>
#include <mbgl/util/vectors.hpp>

namespace mbgl {

// Maps vertex range to feature index
struct FeatureVertexRange {
    std::size_t featureIndex;
    std::size_t start;
    std::size_t end;
};

using FeatureVertexRangeMap = std::map<std::string, std::vector<FeatureVertexRange>>;

/*
   ZoomInterpolatedAttribute<Attr> is a 'compound' attribute, representing two
   values of the the base attribute Attr.  These two values are provided to the
   shader to allow interpolation between zoom levels, without the need to
   repopulate vertex buffers each frame as the map is being zoomed.
*/
template <class AttributeType>
using ZoomInterpolatedAttributeType =
    gfx::AttributeType<typename AttributeType::ElementType, AttributeType::Dimensions * 2>;

template <class A>
using ZoomInterpolatedVertexType = typename gfx::detail::VertexType<ZoomInterpolatedAttributeType<A>>;

inline std::array<float, 1> attributeValue(float v) {
    return {{v}};
}

/*
 * Pack a pair of values, interpreted as uint8's, into a single float.
 * Used to conserve vertex attributes. Values are unpacked in the vertex
 * shader using the `unpack_float()` function, defined in _prelude.vertex.glsl.
 */
template <typename T>
inline uint16_t packUint8Pair(T a, T b) {
    return static_cast<uint16_t>(a) * 256 + static_cast<uint16_t>(b);
}

/*
    Encode a four-component color value into a pair of floats.  Since csscolorparser
    uses 8-bit precision for each color component, for each float we use the upper 8
    bits for one component (e.g. (color.r * 255) * 256), and the lower 8 for another.

    Also note that colors come in as floats 0..1, so we scale by 255.
*/
inline std::array<float, 2> attributeValue(const Color& color) {
    return {{static_cast<float>(packUint8Pair(255 * color.r, 255 * color.g)),
             static_cast<float>(packUint8Pair(255 * color.b, 255 * color.a))}};
}

template <size_t N>
std::array<float, N * 2> zoomInterpolatedAttributeValue(const std::array<float, N>& min,
                                                        const std::array<float, N>& max) {
    std::array<float, N * 2> result;
    for (size_t i = 0; i < N; i++) {
        result[i] = min[i];
        result[i + N] = max[i];
    }
    return result;
}

/*
   PaintPropertyBinder is an abstract class serving as the interface definition
   for the strategy used for constructing, uploading, and binding paint property
   data as GLSL attributes.

   It has three concrete subclasses, one for each of the three strategies we
   use:

   * For _constant_ properties -- those whose value is a constant, or the
   constant result of evaluating a camera function at a particular camera
   position -- we don't need a vertex buffer, and instead use a uniform.
   * For source functions, we use a vertex buffer with a single attribute value,
     the evaluated result of the source function for the given feature.
   * For composite functions, we use a vertex buffer with two attributes: min
   and max values covering the range of zooms at which we expect the tile to be
     displayed. These values are calculated by evaluating the composite function
   for the given feature at strategically chosen zoom levels. In addition to
   this attribute data, we also use a uniform value which the shader uses to
   interpolate between the min and max value at the final displayed zoom level.
   The use of a uniform allows us to cheaply update the value on every frame.

   Note that the shader source varies depending on whether we're using a uniform
   or attribute. Like GL JS, we dynamically compile shaders at runtime to
   accomodate this.
*/

template <class T, class UniformValueType, class PossiblyEvaluatedType, class... As>
class PaintPropertyBinder {
public:
    virtual ~PaintPropertyBinder() = default;

    virtual void populateVertexVector(const GeometryTileFeature& feature,
                                      std::size_t length,
                                      std::size_t index,
                                      const ImagePositions&,
                                      const std::optional<PatternDependency>&,
                                      const CanonicalTileID& canonical,
                                      const style::expression::Value&) = 0;

    virtual void updateVertexVectors(const FeatureStates&, const GeometryTileLayer&, const ImagePositions&) {}

    virtual void updateVertexVector(std::size_t, std::size_t, const GeometryTileFeature&, const FeatureState&) = 0;

#if MLN_LEGACY_RENDERER
    virtual void upload(gfx::UploadPass&) = 0;

    virtual std::tuple<ExpandToType<As, std::optional<gfx::AttributeBinding>>...> attributeBinding(
        const PossiblyEvaluatedType& currentValue) const = 0;
#endif // MLN_LEGACY_RENDERER

    virtual void setPatternParameters(const std::optional<ImagePosition>&,
                                      const std::optional<ImagePosition>&,
                                      const CrossfadeParameters&) = 0;
    virtual std::tuple<ExpandToType<As, float>...> interpolationFactor(float currentZoom) const = 0;
    virtual std::tuple<ExpandToType<As, UniformValueType>...> uniformValue(
        const PossiblyEvaluatedType& currentValue) const = 0;

    virtual std::size_t getVertexCount() const = 0;

    /// Indicates that the vertex data is duplicated for interpolation
    virtual bool isInterpolated() const { return false; }

    virtual std::tuple<ZoomInterpolatedVertexType<As>...> getVertexValue(std::size_t index) const = 0;

    virtual gfx::VertexVectorBasePtr getSharedVertexVector() const = 0;

    static std::unique_ptr<PaintPropertyBinder> create(const PossiblyEvaluatedType& value, float zoom, T defaultValue);

    PaintPropertyStatistics<T> statistics;
};

namespace detail {
const gfx::VertexVectorBasePtr noVector;
}

template <class T, class A>
class ConstantPaintPropertyBinder : public PaintPropertyBinder<T, T, PossiblyEvaluatedPropertyValue<T>, A> {
public:
    ConstantPaintPropertyBinder(T constant_)
        : constant(std::move(constant_)) {}

    void populateVertexVector(const GeometryTileFeature&,
                              std::size_t,
                              std::size_t,
                              const ImagePositions&,
                              const std::optional<PatternDependency>&,
                              const CanonicalTileID&,
                              const style::expression::Value&) override {}
    void updateVertexVector(std::size_t, std::size_t, const GeometryTileFeature&, const FeatureState&) override {}

#if MLN_LEGACY_RENDERER
    void upload(gfx::UploadPass&) override {}

    std::tuple<std::optional<gfx::AttributeBinding>> attributeBinding(
        const PossiblyEvaluatedPropertyValue<T>&) const override {
        return {};
    }
#endif // MLN_LEGACY_RENDERER

    void setPatternParameters(const std::optional<ImagePosition>&,
                              const std::optional<ImagePosition>&,
                              const CrossfadeParameters&) override {}

    std::tuple<float> interpolationFactor(float) const override { return std::tuple<float>{0.0f}; }

    std::tuple<T> uniformValue(const PossiblyEvaluatedPropertyValue<T>& currentValue) const override {
        return std::tuple<T>{currentValue.constantOr(constant)};
    }

    using BaseAttributeType = A;
    using BaseVertex = gfx::VertexType<BaseAttributeType>;

    std::size_t getVertexCount() const override { return 0; }

    std::tuple<ZoomInterpolatedVertexType<A>> getVertexValue(std::size_t) const override {
        return {ZoomInterpolatedVertexType<A>{0}};
    }

    gfx::VertexVectorBasePtr getSharedVertexVector() const override { return detail::noVector; }

private:
    T constant;
    mutable BaseVertex convertedValue;
};

template <class T, class... As>
class ConstantCrossFadedPaintPropertyBinder final
    : public PaintPropertyBinder<T, std::array<uint16_t, 4>, PossiblyEvaluatedPropertyValue<Faded<T>>, As...> {
public:
    ConstantCrossFadedPaintPropertyBinder(Faded<T> constant_)
        : constant(std::move(constant_)),
          constantPatternPositions({}) {}

    void populateVertexVector(const GeometryTileFeature&,
                              std::size_t,
                              std::size_t,
                              const ImagePositions&,
                              const std::optional<PatternDependency>&,
                              const CanonicalTileID&,
                              const style::expression::Value&) override {}
    void updateVertexVector(std::size_t, std::size_t, const GeometryTileFeature&, const FeatureState&) override {}

#if MLN_LEGACY_RENDERER
    void upload(gfx::UploadPass&) override {}

    std::tuple<std::optional<gfx::AttributeBinding>, std::optional<gfx::AttributeBinding>> attributeBinding(
        const PossiblyEvaluatedPropertyValue<Faded<T>>&) const override {
        return {};
    }
#endif // MLN_LEGACY_RENDERER

    void setPatternParameters(const std::optional<ImagePosition>& posA,
                              const std::optional<ImagePosition>& posB,
                              const CrossfadeParameters&) override {
        if (!posA || !posB) {
            return;
        } else {
            constantPatternPositions = std::tuple<std::array<uint16_t, 4>, std::array<uint16_t, 4>>{posB->tlbr(),
                                                                                                    posA->tlbr()};
        }
    }

    std::tuple<float, float> interpolationFactor(float) const override { return std::tuple<float, float>{0.0f, 0.0f}; }

    std::tuple<std::array<uint16_t, 4>, std::array<uint16_t, 4>> uniformValue(
        const PossiblyEvaluatedPropertyValue<Faded<T>>&) const override {
        return constantPatternPositions;
    }

    std::size_t getVertexCount() const override { return 0; }

    std::tuple<ZoomInterpolatedVertexType<As>...> getVertexValue(std::size_t) const override {
        return {ZoomInterpolatedVertexType<As>{0}...};
    }

    gfx::VertexVectorBasePtr getSharedVertexVector() const override { return detail::noVector; }

private:
    Faded<T> constant;
    std::tuple<std::array<uint16_t, 4>, std::array<uint16_t, 4>> constantPatternPositions;
};

template <class T, class A>
class SourceFunctionPaintPropertyBinder final : public PaintPropertyBinder<T, T, PossiblyEvaluatedPropertyValue<T>, A> {
public:
    using BaseAttributeType = A;
    using BaseVertex = gfx::VertexType<BaseAttributeType>;

    SourceFunctionPaintPropertyBinder(style::PropertyExpression<T> expression_, T defaultValue_)
        : expression(std::move(expression_)),
          defaultValue(std::move(defaultValue_)) {}
    ~SourceFunctionPaintPropertyBinder() override { sharedVertexVector->release(); }

    void setPatternParameters(const std::optional<ImagePosition>&,
                              const std::optional<ImagePosition>&,
                              const CrossfadeParameters&) override {}
    void populateVertexVector(const GeometryTileFeature& feature,
                              std::size_t length,
                              std::size_t index,
                              const ImagePositions&,
                              const std::optional<PatternDependency>&,
                              const CanonicalTileID& canonical,
                              const style::expression::Value& formattedSection) override {
        using style::expression::EvaluationContext;
        auto evaluated = expression.evaluate(
            EvaluationContext(&feature).withFormattedSection(&formattedSection).withCanonicalTileID(&canonical),
            defaultValue);
        this->statistics.add(evaluated);
        auto value = attributeValue(evaluated);
        auto elements = vertexVector.elements();
        for (std::size_t i = elements; i < length; ++i) {
            vertexVector.emplace_back(BaseVertex{value});
        }
        std::optional<std::string> idStr = featureIDtoString(feature.getID());
        if (idStr) {
            featureMap[*idStr].emplace_back(FeatureVertexRange{index, elements, length});
        }
    }

    void updateVertexVectors(const FeatureStates& states,
                             const GeometryTileLayer& layer,
                             const ImagePositions&) override {
        for (const auto& it : states) {
            const auto positions = featureMap.find(it.first);
            if (positions == featureMap.end()) {
                continue;
            }

            for (const auto& pos : positions->second) {
                std::unique_ptr<GeometryTileFeature> feature = layer.getFeature(pos.featureIndex);
                if (feature) {
                    updateVertexVector(pos.start, pos.end, *feature, it.second);
                }
            }
        }
    }

    void updateVertexVector(std::size_t start,
                            std::size_t end,
                            const GeometryTileFeature& feature,
                            const FeatureState& state) override {
        using style::expression::EvaluationContext;

        auto evaluated = expression.evaluate(EvaluationContext(&feature).withFeatureState(&state), defaultValue);
        this->statistics.add(evaluated);
        auto value = attributeValue(evaluated);
        for (std::size_t i = start; i < end; ++i) {
            vertexVector.at(i) = BaseVertex{value};
        }
    }

#if MLN_LEGACY_RENDERER
    void upload(gfx::UploadPass& uploadPass) override { vertexBuffer = uploadPass.createVertexBuffer(vertexVector); }

    std::tuple<std::optional<gfx::AttributeBinding>> attributeBinding(
        const PossiblyEvaluatedPropertyValue<T>& currentValue) const override {
        if (currentValue.isConstant()) {
            return {};
        } else {
            return std::tuple<std::optional<gfx::AttributeBinding>>{gfx::attributeBinding(*vertexBuffer)};
        }
    }
#endif // MLN_LEGACY_RENDERER

    std::tuple<float> interpolationFactor(float) const override { return std::tuple<float>{0.0f}; }

    std::tuple<T> uniformValue(const PossiblyEvaluatedPropertyValue<T>& currentValue) const override {
        if (currentValue.isConstant()) {
            return std::tuple<T>{*currentValue.constant()};
        } else {
            // Uniform values for vertex attribute arrays are unused.
            return {};
        }
    }

    std::size_t getVertexCount() const override { return vertexVector.elements(); }

    std::tuple<ZoomInterpolatedVertexType<A>> getVertexValue(std::size_t index) const override {
        const BaseVertex& value = vertexVector.at(index);
        return {ZoomInterpolatedVertexType<A>{concatenate(value.a1, value.a1)}};
    }

    gfx::VertexVectorBasePtr getSharedVertexVector() const override { return sharedVertexVector; }

private:
    style::PropertyExpression<T> expression;
    T defaultValue;

    gfx::VertexVectorPtr<BaseVertex> sharedVertexVector = std::make_shared<gfx::VertexVector<BaseVertex>>();
    gfx::VertexVector<BaseVertex>& vertexVector = *sharedVertexVector;

#if MLN_LEGACY_RENDERER
    std::optional<gfx::VertexBuffer<BaseVertex>> vertexBuffer;
#endif // MLN_LEGACY_RENDERER

    FeatureVertexRangeMap featureMap;
};

template <class T, class A>
class CompositeFunctionPaintPropertyBinder final
    : public PaintPropertyBinder<T, T, PossiblyEvaluatedPropertyValue<T>, A> {
public:
    using AttributeType = ZoomInterpolatedAttributeType<A>;
    using AttributeValue = typename AttributeType::Value;
    using Vertex = gfx::VertexType<AttributeType>;

    CompositeFunctionPaintPropertyBinder(style::PropertyExpression<T> expression_, float zoom, T defaultValue_)
        : expression(std::move(expression_)),
          defaultValue(std::move(defaultValue_)),
          zoomRange({zoom, zoom + 1}) {}
    ~CompositeFunctionPaintPropertyBinder() override { sharedVertexVector->release(); }

    void setPatternParameters(const std::optional<ImagePosition>&,
                              const std::optional<ImagePosition>&,
                              const CrossfadeParameters&) override {}
    void populateVertexVector(const GeometryTileFeature& feature,
                              std::size_t length,
                              std::size_t index,
                              const ImagePositions&,
                              const std::optional<PatternDependency>&,
                              const CanonicalTileID& canonical,
                              const style::expression::Value& formattedSection) override {
        using style::expression::EvaluationContext;
        Range<T> range = {
            expression.evaluate(EvaluationContext(zoomRange.min, &feature)
                                    .withFormattedSection(&formattedSection)
                                    .withCanonicalTileID(&canonical),
                                defaultValue),
            expression.evaluate(EvaluationContext(zoomRange.max, &feature)
                                    .withFormattedSection(&formattedSection)
                                    .withCanonicalTileID(&canonical),
                                defaultValue),
        };
        this->statistics.add(range.min);
        this->statistics.add(range.max);
        const AttributeValue value = zoomInterpolatedAttributeValue(attributeValue(range.min),
                                                                    attributeValue(range.max));
        const auto elements = vertexVector.elements();
        if (vertexVector.empty()) {
            vertexVector.reserve(length);
        }
        for (std::size_t i = elements; i < length; ++i) {
            vertexVector.emplace_back(Vertex{value});
        }
        if (auto idStr = featureIDtoString(feature.getID())) {
            featureMap[*idStr].emplace_back(FeatureVertexRange{index, elements, length});
        }
    }

    void updateVertexVectors(const FeatureStates& states,
                             const GeometryTileLayer& layer,
                             const ImagePositions&) override {
        for (const auto& it : states) {
            const auto positions = featureMap.find(it.first);
            if (positions == featureMap.end()) {
                continue;
            }

            for (const auto& pos : positions->second) {
                std::unique_ptr<GeometryTileFeature> feature = layer.getFeature(pos.featureIndex);
                if (feature) {
                    updateVertexVector(pos.start, pos.end, *feature, it.second);
                }
            }
        }
    }

    void updateVertexVector(std::size_t start,
                            std::size_t end,
                            const GeometryTileFeature& feature,
                            const FeatureState& state) override {
        using style::expression::EvaluationContext;
        Range<T> range = {
            expression.evaluate(EvaluationContext(zoomRange.min, &feature, &state), defaultValue),
            expression.evaluate(EvaluationContext(zoomRange.max, &feature, &state), defaultValue),
        };
        this->statistics.add(range.min);
        this->statistics.add(range.max);
        AttributeValue value = zoomInterpolatedAttributeValue(attributeValue(range.min), attributeValue(range.max));

        for (std::size_t i = start; i < end; ++i) {
            vertexVector.at(i) = Vertex{value};
        }
    }

#if MLN_LEGACY_RENDERER
    void upload(gfx::UploadPass& uploadPass) override { vertexBuffer = uploadPass.createVertexBuffer(vertexVector); }

    std::tuple<std::optional<gfx::AttributeBinding>> attributeBinding(
        const PossiblyEvaluatedPropertyValue<T>& currentValue) const override {
        if (currentValue.isConstant()) {
            return {};
        } else {
            return std::tuple<std::optional<gfx::AttributeBinding>>{gfx::attributeBinding(*vertexBuffer)};
        }
    }
#endif // MLN_LEGACY_RENDERER

    std::tuple<float> interpolationFactor(float currentZoom) const override {
        const float possiblyRoundedZoom = expression.getUseIntegerZoom() ? std::floor(currentZoom) : currentZoom;

        return std::tuple<float>{
            std::fmax(0.0f, std::fmin(1.0f, expression.interpolationFactor(zoomRange, possiblyRoundedZoom)))};
    }

    std::tuple<T> uniformValue(const PossiblyEvaluatedPropertyValue<T>& currentValue) const override {
        if (currentValue.isConstant()) {
            return std::tuple<T>{*currentValue.constant()};
        } else {
            // Uniform values for vertex attribute arrays are unused.
            return {};
        }
    }

    std::size_t getVertexCount() const override { return vertexVector.elements(); }

    bool isInterpolated() const override { return true; }

    gfx::VertexVectorBasePtr getSharedVertexVector() const override { return sharedVertexVector; }

    std::tuple<ZoomInterpolatedVertexType<A>> getVertexValue(std::size_t index) const override {
        return {vertexVector.at(index)};
    }

private:
    style::PropertyExpression<T> expression;
    T defaultValue;
    Range<float> zoomRange;

    gfx::VertexVectorPtr<Vertex> sharedVertexVector = std::make_shared<gfx::VertexVector<Vertex>>();
    gfx::VertexVector<Vertex>& vertexVector = *sharedVertexVector;

#if MLN_LEGACY_RENDERER
    std::optional<gfx::VertexBuffer<Vertex>> vertexBuffer;
#endif // MLN_LEGACY_RENDERER

    FeatureVertexRangeMap featureMap;
};

template <class T, class A1, class A2>
class CompositeCrossFadedPaintPropertyBinder final
    : public PaintPropertyBinder<T, std::array<uint16_t, 4>, PossiblyEvaluatedPropertyValue<Faded<T>>, A1, A2> {
public:
    using BaseAttributeType = A1;
    using BaseAttributeType2 = A2;

    using Vertex = gfx::VertexType<BaseAttributeType>;
    using Vertex2 = gfx::VertexType<BaseAttributeType2>;

    CompositeCrossFadedPaintPropertyBinder(style::PropertyExpression<T> expression_, float zoom, T defaultValue_)
        : expression(std::move(expression_)),
          defaultValue(std::move(defaultValue_)),
          zoomRange({zoom, zoom + 1}) {}
    ~CompositeCrossFadedPaintPropertyBinder() override { sharedPatternToVertexVector->release(); }

    void setPatternParameters(const std::optional<ImagePosition>&,
                              const std::optional<ImagePosition>&,
                              const CrossfadeParameters& crossfade_) override {
        crossfade = crossfade_;
    };

    void populateVertexVector(const GeometryTileFeature&,
                              std::size_t length,
                              std::size_t /* index */,
                              const ImagePositions& patternPositions,
                              const std::optional<PatternDependency>& patternDependencies,
                              const CanonicalTileID&,
                              const style::expression::Value&) override {
        if (!patternDependencies || patternDependencies->mid.empty()) {
            // Unlike other propperties with expressions that evaluate to null,
            // the default value for `*-pattern` properties is an empty string
            // and will not have a valid entry in patternPositions. We still
            // need to populate the attribute buffers to avoid crashes when we
            // try to draw the layer because we don't know at draw time if all
            // features were evaluated to valid pattern dependencies.
            for (std::size_t i = zoomInVertexVector.elements(); i < length; ++i) {
                patternToVertexVector.emplace_back(Vertex{std::array<uint16_t, 4>{{0, 0, 0, 0}}});
                zoomInVertexVector.emplace_back(Vertex2{std::array<uint16_t, 4>{{0, 0, 0, 0}}});
                zoomOutVertexVector.emplace_back(Vertex2{std::array<uint16_t, 4>{{0, 0, 0, 0}}});
            }
        } else if (!patternPositions.empty()) {
            const auto min = patternPositions.find(patternDependencies->min);
            const auto mid = patternPositions.find(patternDependencies->mid);
            const auto max = patternPositions.find(patternDependencies->max);

            const auto end = patternPositions.end();
            if (min == end || mid == end || max == end) return;

            const ImagePosition imageMin = min->second;
            const ImagePosition imageMid = mid->second;
            const ImagePosition imageMax = max->second;

            for (std::size_t i = zoomInVertexVector.elements(); i < length; ++i) {
                patternToVertexVector.emplace_back(Vertex{imageMid.tlbr()});
                zoomInVertexVector.emplace_back(Vertex2{imageMin.tlbr()});
                zoomOutVertexVector.emplace_back(Vertex2{imageMax.tlbr()});
            }
        }
    }

    void updateVertexVector(std::size_t, std::size_t, const GeometryTileFeature&, const FeatureState&) override {}

#if MLN_LEGACY_RENDERER
    void upload(gfx::UploadPass& uploadPass) override {
        if (!patternToVertexVector.empty()) {
            assert(!zoomInVertexVector.empty());
            assert(!zoomOutVertexVector.empty());
            patternToVertexBuffer = uploadPass.createVertexBuffer(patternToVertexVector);
            zoomInVertexBuffer = uploadPass.createVertexBuffer(zoomInVertexVector);
            zoomOutVertexBuffer = uploadPass.createVertexBuffer(zoomOutVertexVector);
        }
    }

    std::tuple<std::optional<gfx::AttributeBinding>, std::optional<gfx::AttributeBinding>> attributeBinding(
        const PossiblyEvaluatedPropertyValue<Faded<T>>& currentValue) const override {
        if (currentValue.isConstant()) {
            return {};
        } else {
            if (patternToVertexBuffer) {
                assert(zoomInVertexBuffer);
                assert(zoomOutVertexBuffer);
                return std::tuple<std::optional<gfx::AttributeBinding>, std::optional<gfx::AttributeBinding>>{
                    gfx::attributeBinding(*patternToVertexBuffer),
                    gfx::attributeBinding(crossfade.fromScale == 2 ? *zoomInVertexBuffer : *zoomOutVertexBuffer)};
            }

            return std::tuple<std::optional<gfx::AttributeBinding>, std::optional<gfx::AttributeBinding>>{std::nullopt,
                                                                                                          std::nullopt};
        }
    }
#endif // MLN_LEGACY_RENDERER

    std::tuple<float, float> interpolationFactor(float) const override { return std::tuple<float, float>{0.0f, 0.0f}; }

    std::tuple<std::array<uint16_t, 4>, std::array<uint16_t, 4>> uniformValue(
        const PossiblyEvaluatedPropertyValue<Faded<T>>&) const override {
        // Uniform values for vertex attribute arrays are unused.
        return {};
    }

    std::size_t getVertexCount() const override { return patternToVertexVector.elements(); }

    std::tuple<ZoomInterpolatedVertexType<A1>, ZoomInterpolatedVertexType<A2>> getVertexValue(
        std::size_t index) const override {
        const Vertex& patternValue = patternToVertexVector.at(index);
        const Vertex2& zoomValue = crossfade.fromScale == 2 ? zoomInVertexVector.at(index)
                                                            : zoomOutVertexVector.at(index);
        return {ZoomInterpolatedVertexType<A1>{concatenate(patternValue.a1, patternValue.a1)},
                ZoomInterpolatedVertexType<A2>{concatenate(zoomValue.a1, zoomValue.a1)}};
    }

    gfx::VertexVectorBasePtr getSharedVertexVector() const override { return sharedPatternToVertexVector; }

private:
    style::PropertyExpression<T> expression;
    T defaultValue;
    Range<float> zoomRange;

    gfx::VertexVectorPtr<Vertex> sharedPatternToVertexVector = std::make_shared<gfx::VertexVector<Vertex>>();
    gfx::VertexVector<Vertex>& patternToVertexVector = *sharedPatternToVertexVector;

    gfx::VertexVector<Vertex2> zoomInVertexVector;
    gfx::VertexVector<Vertex2> zoomOutVertexVector;

#if MLN_LEGACY_RENDERER
    std::optional<gfx::VertexBuffer<Vertex>> patternToVertexBuffer;
    std::optional<gfx::VertexBuffer<Vertex2>> zoomInVertexBuffer;
    std::optional<gfx::VertexBuffer<Vertex2>> zoomOutVertexBuffer;
#endif // MLN_LEGACY_RENDERER

    CrossfadeParameters crossfade;
};

template <class T, class PossiblyEvaluatedType>
struct CreateBinder {
    template <class A>
    static std::unique_ptr<PaintPropertyBinder<T, T, PossiblyEvaluatedType, A>> create(
        const PossiblyEvaluatedType& value, float zoom, T defaultValue) {
        return value.match(
            [&](const T& constant) -> std::unique_ptr<PaintPropertyBinder<T, T, PossiblyEvaluatedType, A>> {
                return std::make_unique<ConstantPaintPropertyBinder<T, A>>(constant);
            },
            [&](const style::PropertyExpression<T>& expression)
                -> std::unique_ptr<PaintPropertyBinder<T, T, PossiblyEvaluatedType, A>> {
                if (expression.isZoomConstant()) {
                    return std::make_unique<SourceFunctionPaintPropertyBinder<T, A>>(expression, defaultValue);
                } else {
                    return std::make_unique<CompositeFunctionPaintPropertyBinder<T, A>>(expression, zoom, defaultValue);
                }
            });
    }
};

template <class T>
struct CreateBinder<T, PossiblyEvaluatedPropertyValue<Faded<T>>> {
    template <class A1, class A2>
    static std::unique_ptr<
        PaintPropertyBinder<T, std::array<uint16_t, 4>, PossiblyEvaluatedPropertyValue<Faded<T>>, A1, A2>>
    create(const PossiblyEvaluatedPropertyValue<Faded<T>>& value, float zoom, T defaultValue) {
        return value.match(
            [&](const Faded<T>& constant)
                -> std::unique_ptr<
                    PaintPropertyBinder<T, std::array<uint16_t, 4>, PossiblyEvaluatedPropertyValue<Faded<T>>, A1, A2>> {
                return std::make_unique<ConstantCrossFadedPaintPropertyBinder<T, A1, A2>>(constant);
            },
            [&](const style::PropertyExpression<T>& expression)
                -> std::unique_ptr<
                    PaintPropertyBinder<T, std::array<uint16_t, 4>, PossiblyEvaluatedPropertyValue<Faded<T>>, A1, A2>> {
                return std::make_unique<CompositeCrossFadedPaintPropertyBinder<T, A1, A2>>(
                    expression, zoom, defaultValue);
            });
    }
};

template <class T, class UniformValueType, class PossiblyEvaluatedType, class... As>
std::unique_ptr<PaintPropertyBinder<T, UniformValueType, PossiblyEvaluatedType, As...>>
PaintPropertyBinder<T, UniformValueType, PossiblyEvaluatedType, As...>::create(const PossiblyEvaluatedType& value,
                                                                               float zoom,
                                                                               T defaultValue) {
    return CreateBinder<T, PossiblyEvaluatedType>::template create<As...>(value, zoom, defaultValue);
}

template <class Attr>
struct ZoomInterpolatedAttribute : public Attr {
    using Type = ZoomInterpolatedAttributeType<typename Attr::Type>;
};

template <class Attr>
struct InterpolationUniform {
    using Value = float;
    static constexpr auto name() { return concat_literals<&Attr::name, &string_literal<'_', 't'>::value>::value(); }
};

template <class Ps>
class PaintPropertyBinders;

template <class... Ps>
class PaintPropertyBinders<TypeList<Ps...>> {
private:
    template <class T, class PossiblyEvaluatedType, class... As>
    struct Detail;

    template <class T, class UniformValueType, class PossiblyEvaluatedType, class... As>
    struct Detail<T, UniformValueType, PossiblyEvaluatedType, TypeList<As...>> {
        using Binder = PaintPropertyBinder<T, UniformValueType, PossiblyEvaluatedType, typename As::Type...>;
        using ZoomInterpolatedAttributeList = TypeList<ZoomInterpolatedAttribute<As>...>;
        using InterpolationUniformList = TypeList<InterpolationUniform<As>...>;
    };

    template <class P>
    using Property = Detail<typename P::Type,
                            typename P::Uniform::Value,
                            typename P::PossiblyEvaluatedType,
                            typename P::AttributeList>;

public:
    template <class P>
    using Binder = typename Property<P>::Binder;

    using Binders = IndexedTuple<TypeList<Ps...>, TypeList<std::unique_ptr<Binder<Ps>>...>>;

    template <class EvaluatedProperties>
    PaintPropertyBinders(const EvaluatedProperties& properties, float z)
        : binders(Binder<Ps>::create(properties.template get<Ps>(), z, Ps::defaultValue())...) {
        (void)z; // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56958
    }

    PaintPropertyBinders(PaintPropertyBinders&&) noexcept = default;
    PaintPropertyBinders(const PaintPropertyBinders&) = delete;

    void populateVertexVectors(const GeometryTileFeature& feature,
                               std::size_t length,
                               std::size_t index,
                               const ImagePositions& patternPositions,
                               const std::optional<PatternDependency>& patternDependencies,
                               const CanonicalTileID& canonical,
                               const style::expression::Value& formattedSection = {}) {
        util::ignore({(binders.template get<Ps>()->populateVertexVector(
                           feature, length, index, patternPositions, patternDependencies, canonical, formattedSection),
                       0)...});
    }

    void updateVertexVectors(const FeatureStates& states,
                             const GeometryTileLayer& layer,
                             const ImagePositions& imagePositions) {
        util::ignore({(binders.template get<Ps>()->updateVertexVectors(states, layer, imagePositions), 0)...});
    }

    void setPatternParameters(const std::optional<ImagePosition>& posA,
                              const std::optional<ImagePosition>& posB,
                              const CrossfadeParameters& crossfade) const {
        util::ignore({(binders.template get<Ps>()->setPatternParameters(posA, posB, crossfade), 0)...});
    }

#if MLN_LEGACY_RENDERER
    void upload(gfx::UploadPass& uploadPass) { util::ignore({(binders.template get<Ps>()->upload(uploadPass), 0)...}); }
#endif // MLN_LEGACY_RENDERER

    template <class P>
    using ZoomInterpolatedAttributeList = typename Property<P>::ZoomInterpolatedAttributeList;
    template <class P>
    using InterpolationUniformList = typename Property<P>::InterpolationUniformList;

    using AttributeList = TypeListConcat<ZoomInterpolatedAttributeList<Ps>...>;
    using AttributeBindings = gfx::AttributeBindings<AttributeList>;

    template <class EvaluatedProperties>
    AttributeBindings attributeBindings(const EvaluatedProperties& currentProperties) const {
        return AttributeBindings{
            std::tuple_cat(binders.template get<Ps>()->attributeBinding(currentProperties.template get<Ps>())...)};
    }

    using UniformList = TypeListConcat<InterpolationUniformList<Ps>..., typename Ps::UniformList...>;
    using UniformValues = gfx::UniformValues<UniformList>;

    template <class EvaluatedProperties>
    UniformValues uniformValues(float currentZoom, EvaluatedProperties& currentProperties) const {
        (void)currentZoom; // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56958
        return UniformValues(std::tuple_cat(
            // interpolation uniform values
            binders.template get<Ps>()->interpolationFactor(currentZoom)...,
            // uniform values
            binders.template get<Ps>()->uniformValue(currentProperties.template get<Ps>())...));
    }

    template <class P>
    const auto& get() const {
        return binders.template get<P>();
    }

    template <class P>
    const auto& statistics() const {
        return binders.template get<P>()->statistics;
    }

private:
    Binders binders;
};

} // namespace mbgl
