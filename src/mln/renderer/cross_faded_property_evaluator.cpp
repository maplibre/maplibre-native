#include <mln/renderer/cross_faded_property_evaluator.hpp>
#include <mln/style/expression/image.hpp>
#include <mln/util/chrono.hpp>

#include <cmath>

namespace mln {

template <typename T>
Faded<T> CrossFadedPropertyEvaluator<T>::operator()(const style::Undefined&) const {
    return calculate(defaultValue, defaultValue, defaultValue);
}

template <typename T>
Faded<T> CrossFadedPropertyEvaluator<T>::operator()(const T& constant) const {
    return calculate(constant, constant, constant);
}

template <typename T>
Faded<T> CrossFadedPropertyEvaluator<T>::operator()(const style::PropertyExpression<T>& expression) const {
    using style::expression::EvaluationContext;
    const auto* globalState = parameters.globalState.get();
    return calculate(expression.evaluate(EvaluationContext(parameters.z - 1.0f).withGlobalState(globalState)),
                     expression.evaluate(EvaluationContext(parameters.z).withGlobalState(globalState)),
                     expression.evaluate(EvaluationContext(parameters.z + 1.0f).withGlobalState(globalState)));
}

template <typename T>
Faded<T> CrossFadedPropertyEvaluator<T>::calculate(const T& min, const T& mid, const T& max) const {
    const float z = parameters.z;
    return z > parameters.zoomHistory.lastIntegerZoom ? Faded<T>{min, mid} : Faded<T>{max, mid};
}

template class CrossFadedPropertyEvaluator<style::expression::Image>;
template class CrossFadedPropertyEvaluator<std::vector<float>>;

} // namespace mln
