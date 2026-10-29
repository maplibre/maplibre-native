#include <mln/test/util.hpp>

#include <mln/style/properties.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/property_value.hpp>
#include <mln/style/expression/dsl.hpp>
#include <mln/style/light_impl.hpp>
#include <mln/renderer/property_evaluator.hpp>
#include <mln/renderer/data_driven_property_evaluator.hpp>

using namespace mln;
using namespace mln::style;
using namespace std::literals::chrono_literals;

float evaluate(Transitioning<PropertyValue<float>>& property, Duration delta = Duration::zero()) {
    ZoomHistory zoomHistory;
    zoomHistory.update(0, TimePoint::min() + delta);

    PropertyEvaluationParameters parameters{zoomHistory, TimePoint::min() + delta, Duration::zero()};

    PropertyEvaluator<float> evaluator{parameters, 0.0f};

    return property.evaluate(evaluator, parameters.now);
}

PossiblyEvaluatedPropertyValue<float> evaluateDataExpression(Transitioning<PropertyValue<float>>& property,
                                                             Duration delta = Duration::zero()) {
    ZoomHistory zoomHistory;
    zoomHistory.update(0, TimePoint::min() + delta);

    PropertyEvaluationParameters parameters{zoomHistory, TimePoint::min() + delta, Duration::zero()};

    DataDrivenPropertyEvaluator<float> evaluator{parameters, 0.0f};

    return property.evaluate(evaluator, parameters.now);
}

TEST(TransitioningPropertyValue, EvaluateDefaultValue) {
    Transitioning<PropertyValue<float>> property;
    ASSERT_EQ(0.0f, evaluate(property));
}

TEST(TransitioningPropertyValue, EvaluateUntransitionedConstant) {
    Transitioning<PropertyValue<float>> property{
        PropertyValue<float>(1.0f), Transitioning<PropertyValue<float>>(), TransitionOptions(), TimePoint::min()};

    ASSERT_EQ(1.0f, evaluate(property));
}

TEST(TransitioningPropertyValue, EvaluateTransitionedConstantWithoutDelay) {
    TransitionOptions transition;
    transition.duration = {1000ms};

    Transitioning<PropertyValue<float>> t0{
        PropertyValue<float>(0.0f), Transitioning<PropertyValue<float>>(), TransitionOptions(), TimePoint::min()};

    Transitioning<PropertyValue<float>> t1{PropertyValue<float>(1.0f), t0, transition, TimePoint::min()};

    ASSERT_FLOAT_EQ(0.0f, evaluate(t1, 0ms));
    ASSERT_FLOAT_EQ(0.823099f, evaluate(t1, 500ms));
    ASSERT_FLOAT_EQ(1.0f, evaluate(t1, 1500ms));
}

TEST(TransitioningPropertyValue, EvaluateTransitionedConstantWithDelay) {
    TransitionOptions transition;
    transition.delay = {1000ms};
    transition.duration = {1000ms};

    Transitioning<PropertyValue<float>> t0{
        PropertyValue<float>(0.0f), Transitioning<PropertyValue<float>>(), TransitionOptions(), TimePoint::min()};

    Transitioning<PropertyValue<float>> t1{PropertyValue<float>(1.0f), t0, transition, TimePoint::min()};

    ASSERT_FLOAT_EQ(0.0f, evaluate(t1, 0ms));
    ASSERT_FLOAT_EQ(0.0f, evaluate(t1, 500ms));
    ASSERT_FLOAT_EQ(0.0f, evaluate(t1, 612ms));
    ASSERT_FLOAT_EQ(0.823099f, evaluate(t1, 1500ms));
    ASSERT_FLOAT_EQ(1.0f, evaluate(t1, 2500ms));
}

TEST(TransitioningDataDrivenPropertyValue, Evaluate) {
    TransitionOptions transition;
    transition.delay = {1000ms};
    transition.duration = {1000ms};

    Transitioning<PropertyValue<float>> t0{
        PropertyValue<float>(0.0f), Transitioning<PropertyValue<float>>(), TransitionOptions(), TimePoint::min()};

    using namespace mln::style::expression::dsl;
    PropertyExpression<float> expression(number(get("property_name")));

    Transitioning<PropertyValue<float>> t1{PropertyValue<float>(expression), t0, transition, TimePoint::min()};

    ASSERT_TRUE(evaluateDataExpression(t0, 0ms).isConstant());
    ASSERT_FALSE(evaluateDataExpression(t1, 0ms).isConstant())
        << "A paint property transition to a data-driven evaluates immediately "
           "to the final value (see "
           "https://github.com/mapbox/mapbox-gl-native/issues/8237).";
}

namespace {

PropertyValue<float> globalStateValue(const std::string& key) {
    conversion::Error error;
    auto value = conversion::convertJSON<PropertyValue<float>>(
        R"(["global-state", ")" + key + R"("])", error, false, false);
    EXPECT_TRUE(value) << error.message;
    return *value;
}

std::shared_ptr<const GlobalStateMap> widthState(double width) {
    return std::make_shared<const GlobalStateMap>(GlobalStateMap{{"width", width}});
}

float evaluate(const Transitioning<PropertyValue<float>>& property,
               std::shared_ptr<const GlobalStateMap> globalState,
               Duration delta) {
    ZoomHistory zoomHistory;
    zoomHistory.update(0, TimePoint::min() + delta);

    PropertyEvaluationParameters parameters{zoomHistory, TimePoint::min() + delta, Duration::zero()};
    parameters.globalState = std::move(globalState);

    PropertyEvaluator<float> evaluator{parameters, 0.0f};

    return property.evaluate(evaluator, parameters.now);
}

TransitionParameters globalStateChange(std::shared_ptr<const GlobalStateMap> priorState,
                                       const std::string& changedKey,
                                       Duration delta,
                                       bool stateOnly = true) {
    TransitionOptions transition;
    transition.duration = {1000ms};
    return {.now = TimePoint::min() + delta,
            .transition = transition,
            .globalStateChange = GlobalStateChange{
                .priorState = std::move(priorState),
                .changedKeys = std::make_shared<const std::set<std::string>>(std::set<std::string>{changedKey}),
                .stateOnly = stateOnly}};
}

} // namespace

TEST(TransitioningPropertyValue, GlobalStateChangeTransitionsFromPriorState) {
    const Transitionable<PropertyValue<float>> width{.value = globalStateValue("width"), .options = {}};
    auto property = width.transition({.now = TimePoint::min()}, {});
    ASSERT_FLOAT_EQ(0.0f, evaluate(property, widthState(0), 0ms));

    property = width.transition(globalStateChange(widthState(0), "width", 0ms), std::move(property));

    EXPECT_FLOAT_EQ(0.0f, evaluate(property, widthState(1), 0ms));
    EXPECT_FLOAT_EQ(0.823099f, evaluate(property, widthState(1), 500ms));
    EXPECT_FLOAT_EQ(1.0f, evaluate(property, widthState(1), 1500ms));
}

TEST(TransitioningPropertyValue, GlobalStateChangeContinuesRunningTransition) {
    const Transitionable<PropertyValue<float>> width{.value = globalStateValue("width"), .options = {}};
    auto property = width.transition({.now = TimePoint::min()}, {});
    property = width.transition(globalStateChange(widthState(0), "width", 0ms), std::move(property));
    ASSERT_FLOAT_EQ(0.823099f, evaluate(property, widthState(1), 500ms));

    // Changing the state back halfway starts from the value shown, not from the state now read.
    property = width.transition(globalStateChange(widthState(1), "width", 500ms), std::move(property));

    EXPECT_FLOAT_EQ(0.823099f, evaluate(property, widthState(0), 500ms));
    EXPECT_FLOAT_EQ(0.0f, evaluate(property, widthState(0), 2500ms));
}

TEST(TransitioningPropertyValue, GlobalStateChangeWithLayerChangeTransitionsFromPriorState) {
    const Transitionable<PropertyValue<float>> width{.value = globalStateValue("width"), .options = {}};
    auto property = width.transition({.now = TimePoint::min()}, {});
    property = width.transition(globalStateChange(widthState(0), "width", 0ms), std::move(property));
    ASSERT_FLOAT_EQ(0.823099f, evaluate(property, widthState(1), 500ms));

    // A layer change in the same frame as the state change restarts the transition from the value shown.
    property = width.transition(globalStateChange(widthState(1), "width", 500ms, false), std::move(property));

    EXPECT_FLOAT_EQ(0.823099f, evaluate(property, widthState(0), 500ms));
    EXPECT_FLOAT_EQ(0.0f, evaluate(property, widthState(0), 2500ms));
}

TEST(TransitioningPropertyValue, GlobalStateChangeSnapsPatternImage) {
    conversion::Error error;
    const auto pattern = conversion::convertJSON<PropertyValue<expression::Image>>(
        R"(["image", ["global-state", "pattern"]])", error, true, false);
    ASSERT_TRUE(pattern) << error.message;
    const Transitionable<PropertyValue<expression::Image>> value{.value = *pattern, .options = {}};
    auto property = value.transition({.now = TimePoint::min()}, {});

    // Pattern images are laid out into tiles, not interpolated, so the state change applies at once.
    const auto priorState = std::make_shared<const GlobalStateMap>(GlobalStateMap{{"pattern", std::string("a")}});
    property = value.transition(globalStateChange(priorState, "pattern", 0ms), std::move(property));

    EXPECT_FALSE(property.hasTransition());
}

TEST(TransitioningPropertyValue, GlobalStateChangeOfUnreadKeyKeepsValue) {
    const Transitionable<PropertyValue<float>> width{.value = globalStateValue("width"), .options = {}};
    auto property = width.transition({.now = TimePoint::min()}, {});

    property = width.transition(globalStateChange(widthState(0), "color", 0ms), std::move(property));

    EXPECT_FALSE(property.hasTransition());
    EXPECT_FLOAT_EQ(1.0f, evaluate(property, widthState(1), 0ms));
}

TEST(TransitioningPropertyValue, GlobalStateChangeTransitionsLightProperty) {
    LightProperties::Transitionable light;
    light.get<LightIntensity>().value = globalStateValue("width");
    auto transitioning = light.transitioned({.now = TimePoint::min()}, light.untransitioned());

    // A state-only change transitions the property that reads the key and leaves the others alone.
    transitioning = light.transitioned(globalStateChange(widthState(0), "width", 0ms), std::move(transitioning));

    EXPECT_TRUE(transitioning.get<LightIntensity>().hasTransition());
    EXPECT_FALSE(transitioning.get<LightColor>().hasTransition());
    EXPECT_FLOAT_EQ(0.0f, evaluate(transitioning.get<LightIntensity>(), widthState(1), 0ms));
    EXPECT_FLOAT_EQ(0.823099f, evaluate(transitioning.get<LightIntensity>(), widthState(1), 500ms));
    EXPECT_FLOAT_EQ(1.0f, evaluate(transitioning.get<LightIntensity>(), widthState(1), 1500ms));
}
