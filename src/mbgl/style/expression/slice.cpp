#include "mbgl/style/expression/expression.hpp"
#include <limits>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/style/expression/slice.hpp>
#include <mbgl/util/string.hpp>

namespace mbgl {
namespace style {
namespace expression {

namespace {

int normalizeIndex(int index, int length) {
    if (index < 0) {
        index += length;
    }
    return index;
}

} // namespace

Slice::Slice(std::unique_ptr<Expression> input_,
             std::unique_ptr<Expression> fromIndex_,
             std::unique_ptr<Expression> toIndex_)
    : Expression(Kind::Slice, input_->getType()),
      input(std::move(input_)),
      fromIndex(std::move(fromIndex_)),
      toIndex(std::move(toIndex_)) {}

EvaluationResult Slice::evaluate(const EvaluationContext &params) const {
    const EvaluationResult evaluatedInput = input->evaluate(params);
    if (!evaluatedInput) {
        return evaluatedInput.error();
    }
    const EvaluationResult evaluatedFromIndex = fromIndex->evaluate(params);
    if (!fromIndex) {
        return evaluatedFromIndex.error();
    }
    if (!evaluatedFromIndex->is<double>()) {
        return EvaluationError{"Expected value to be of type number, but found " +
                               toString(typeOf(*evaluatedFromIndex)) + " instead."};
    }

    int fromIndexValue = static_cast<int>(evaluatedFromIndex->get<double>());
    int toIndexValue = std::numeric_limits<int>::max();

    if (toIndex) {
        const EvaluationResult evaluatedToIndex = toIndex->evaluate(params);
        if (!evaluatedToIndex) {
            return evaluatedToIndex.error();
        }
        if (!evaluatedToIndex->is<double>()) {
            return EvaluationError{"Expected value to be of type number, but found " +
                                   toString(typeOf(*evaluatedFromIndex)) + " instead."};
        }
        toIndexValue = static_cast<int>(evaluatedToIndex->get<double>());
    }

    return evaluatedInput->match(
        [&](const std::string &s) { return evaluateForStringInput(s, fromIndexValue, toIndexValue); },
        [&](const std::vector<Value> &v) { return evaluateForArrayInput(v, fromIndexValue, toIndexValue); },
        [&](const auto &) -> EvaluationResult {
            return EvaluationError{"Expected first argument to be of type array or string, but found " +
                                   toString(typeOf(*evaluatedInput)) + " instead."};
        });
}

EvaluationResult Slice::evaluateForArrayInput(const std::vector<Value> &array,
                                              int fromIndexValue,
                                              int toIndexValue) const {
    int length = static_cast<int>(array.size());
    if (toIndexValue == std::numeric_limits<int>::max()) {
        toIndexValue = length;
    }
    fromIndexValue = normalizeIndex(fromIndexValue, length);
    toIndexValue = normalizeIndex(toIndexValue, length);

    if (fromIndexValue >= toIndexValue) {
        return std::vector<Value>{};
    }
    toIndexValue = std::min(toIndexValue, length);

    std::vector<Value> result;
    result.reserve(toIndexValue - fromIndexValue);
    for (int index = fromIndexValue; index < toIndexValue; ++index) {
        result.push_back(array[index]);
    }
    return result;
}

EvaluationResult Slice::evaluateForStringInput(const std::string &string, int fromIndexValue, int toIndexValue) const {
    int length = static_cast<int>(string.size());
    if (toIndexValue == std::numeric_limits<int>::max()) {
        toIndexValue = length;
    }
    fromIndexValue = normalizeIndex(fromIndexValue, length);
    toIndexValue = normalizeIndex(toIndexValue, length);
    if (fromIndexValue >= length) {
        return std::string{};
    }

    return string.substr(fromIndexValue, toIndexValue - fromIndexValue);
}

void Slice::eachChild(const std::function<void(const Expression &)> &visit) const {
    visit(*input);
    visit(*fromIndex);
    if (toIndex) {
        visit(*toIndex);
    }
}

using namespace mbgl::style::conversion;
ParseResult Slice::parse(const Convertible &value, ParsingContext &ctx) {
    assert(isArray(value));

    std::size_t length = arrayLength(value);
    if (length != 3 && length != 4) {
        ctx.error("Expected 2 or 3 arguments, but found " + util::toString(length - 1) + " instead.");
        return ParseResult();
    }

    ParseResult input = ctx.parse(arrayMember(value, 1), 1);
    ParseResult fromIndex = ctx.parse(arrayMember(value, 2), 2);

    ParseResult toIndex = length == 4 ? ctx.parse(arrayMember(value, 3), 3) : ParseResult();

    if (!input || !fromIndex) {
        return ParseResult();
    }

    return ParseResult(
        std::make_unique<Slice>(std::move(*input), std::move(*fromIndex), toIndex ? std::move(*toIndex) : nullptr));
}

bool Slice::operator==(const Expression &e) const {
    if (e.getKind() == Kind::Slice) {
        auto rhs = static_cast<const Slice *>(&e);
        const auto toIndexEqual = (toIndex && rhs->toIndex && *toIndex == *(rhs->toIndex)) ||
                                  (!toIndex && !rhs->toIndex);
        return *input == *(rhs->input) && *fromIndex == *(rhs->fromIndex) && toIndexEqual;
    }
    return false;
}

std::vector<optional<Value>> Slice::possibleOutputs() const {
    return {nullopt};
}

std::string Slice::getOperator() const {
    return "slice";
}

} // namespace expression
} // namespace style
} // namespace mbgl
