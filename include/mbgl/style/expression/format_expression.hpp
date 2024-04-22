#pragma once

#include <mbgl/style/expression/expression.hpp>
#include <mbgl/style/expression/parsing_context.hpp>

namespace mbgl {
namespace style {
namespace expression {

struct FormatExpressionSection {
    explicit FormatExpressionSection(std::unique_ptr<Expression> content_);

    void setTextSectionOptions(std::unique_ptr<Expression>&& fontScale_,
                               std::unique_ptr<Expression>&& textFont_,
                               std::unique_ptr<Expression>&& textColor_);

    // Content can be expression that evaluates to String or Image.
    std::shared_ptr<Expression> content;

    // Text related section options.
    std::shared_ptr<Expression> fontScale;
    std::shared_ptr<Expression> textFont;
    std::shared_ptr<Expression> textColor;
};

class FormatExpression final : public Expression {
public:
    explicit FormatExpression(std::vector<FormatExpressionSection> sections);

    EvaluationResult evaluate(const EvaluationContext&) const override;
    static ParseResult parse(const mbgl::style::conversion::Convertible&, ParsingContext&);

    void eachChild(const std::function<void(const Expression&)>&) const override;

    bool operator==(const Expression& e) const noexcept override;

    std::vector<std::optional<Value>> possibleOutputs() const noexcept override {
        // Technically the combinatoric set of all children
        // Usually, this.text will be undefined anyway
        return {std::nullopt};
    }

    const std::vector<FormatExpressionSection>& getSections() const noexcept { return sections; }

    mbgl::Value serialize() const override;
    std::string getOperator() const override { return "format"; }

private:
    static Dependency depsOfSection(const FormatExpressionSection& s) {
        return depsOf(s.content) | depsOf(s.fontScale) | depsOf(s.textFont) | depsOf(s.textColor);
    }

private:
    std::vector<FormatExpressionSection> sections;
};

} // namespace expression
} // namespace style
} // namespace mbgl
