#include "mbgl/style/expression/expression.hpp"
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/style/expression/index_of.hpp>
#include <mbgl/util/string.hpp>
#include <iostream>
namespace mbgl {
namespace style {
namespace expression {

IndexOf::IndexOf(std::unique_ptr<Expression> keyword_,
                 std::unique_ptr<Expression> input_,
                 std::unique_ptr<Expression> fromIndex_)
    : Expression(Kind::IndexOf, type::Number),
      keyword(std::move(keyword_)),
      input(std::move(input_)),
      fromIndex(std::move(fromIndex_)) {}

EvaluationResult IndexOf::evaluate(const EvaluationContext &params) const {
    const EvaluationResult evaluatedKeyword = keyword->evaluate(params);
    const EvaluationResult evaluatedInput = input->evaluate(params);
    if (!evaluatedKeyword) {
        return evaluatedKeyword.error();
    }
    if (!evaluatedInput) {
        return evaluatedInput.error();
    }

    if (!(evaluatedKeyword->is<double>() || evaluatedKeyword->is<std::string>() || evaluatedKeyword->is<bool>() ||
          evaluatedKeyword->is<NullValue>())) {
        return EvaluationError{"Expected first argument to be of type boolean, string, number or null, but found " +
                               toString(typeOf(*evaluatedKeyword)) + " instead."};
    }

    size_t fromIndexValue = 0;
    if (fromIndex) {
        const EvaluationResult evaluatedFromIndex = fromIndex->evaluate(params);
        if (!evaluatedFromIndex) {
            return evaluatedFromIndex.error();
        }
        if (!evaluatedFromIndex->is<double>()) {
            return EvaluationError{"Expected third argument to be of type number, but found " +
                                   toString(typeOf(*evaluatedFromIndex)) + " instead."};
        }
        fromIndexValue = static_cast<size_t>(std::max(evaluatedFromIndex->get<double>(), 0.0));
    }

    return evaluatedInput->match(
        [&](const std::string &s) { return evaluateForStringInput(s, *evaluatedKeyword, fromIndexValue); },
        [&](const std::vector<Value> &v) { return evaluateForArrayInput(v, *evaluatedKeyword, fromIndexValue); },
        [&](const auto &) -> EvaluationResult {
            return EvaluationError{"Expected second argument to be of type array or string, but found " +
                                   toString(typeOf(*evaluatedInput)) + " instead."};
        });
}

EvaluationResult IndexOf::evaluateForArrayInput(const std::vector<Value> &array,
                                                const Value &keywordValue,
                                                size_t fromIndexValue) const {
    for (size_t index = fromIndexValue; index < array.size(); ++index) {
        if (array[index] == keywordValue) {
            return static_cast<double>(index);
        }
    }
    return static_cast<double>(-1);
}

EvaluationResult IndexOf::evaluateForStringInput(const std::string &string,
                                                 const Value &keywordValue,
                                                 size_t fromIndexValue) const {
    std::string keywordString = keywordValue.match(
        [](const std::string &s) { return s; },
        [](bool b) { return b ? std::string{"true"} : std::string{"false"}; },
        [](NullValue) { return std::string{"null"}; },
        [](double n) { return util::toString(n); },
        [](const auto &) -> std::string {
            // it should be impossible to get here - we do validation in evaluate()
            assert(false);
            return "";
        });
    size_t index = string.find(keywordString, fromIndexValue);
    if (index == std::string::npos) {
        return static_cast<double>(-1);
    }
    return static_cast<double>(index);
}

void IndexOf::eachChild(const std::function<void(const Expression &)> &visit) const {
    visit(*keyword);
    visit(*input);
    if (fromIndex) {
        visit(*fromIndex);
    }
}

using namespace mbgl::style::conversion;
ParseResult IndexOf::parse(const Convertible &value, ParsingContext &ctx) {
    assert(isArray(value));

    std::size_t length = arrayLength(value);
    if (length != 3 && length != 4) {
        ctx.error("Expected 2 or 3 arguments, but found " + util::toString(length - 1) + " instead.");
        return ParseResult();
    }

    ParseResult keyword = ctx.parse(arrayMember(value, 1), 1);
    ParseResult input = ctx.parse(arrayMember(value, 2), 2);

    ParseResult fromIndex = length == 4 ? ctx.parse(arrayMember(value, 3), 3) : ParseResult();

    if (!keyword || !input) {
        return ParseResult();
    }

    return ParseResult(
        std::make_unique<IndexOf>(std::move(*keyword), std::move(*input), fromIndex ? std::move(*fromIndex) : nullptr));
}

bool IndexOf::operator==(const Expression &e) const {
    if (e.getKind() == Kind::IndexOf) {
        auto rhs = static_cast<const IndexOf *>(&e);
        const auto fromIndexEqual = (fromIndex && rhs->fromIndex && *fromIndex == *(rhs->fromIndex)) ||
                                    (!fromIndex && !rhs->fromIndex);
        return *keyword == *(rhs->keyword) && *input == *(rhs->input) && fromIndexEqual;
    }
    return false;
}

std::vector<std::optional<Value>> IndexOf::possibleOutputs() const {
    return {std::nullopt};
}

std::string IndexOf::getOperator() const {
    return "index-of";
}

} // namespace expression
} // namespace style
} // namespace mbgl
