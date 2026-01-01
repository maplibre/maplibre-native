#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/style/rapidjson_conversion.hpp>
#include <mbgl/style/expression/dsl.hpp>
#include <mbgl/style/expression/is_expression.hpp>
#include <mbgl/test/util.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/util/rapidjson.hpp>
#include <rapidjson/document.h>
#include <iostream>
#include <filesystem>
#include <fstream>

using namespace mbgl;
using namespace mbgl::style;

TEST(Expression, IsExpression) {
    rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator> spec;
    spec.Parse<0>(util::read_file("scripts/style-spec-reference/v8.json").c_str());
    ASSERT_FALSE(spec.HasParseError());
    ASSERT_TRUE(spec.IsObject() && spec.HasMember("expression_name") && spec["expression_name"].IsObject() &&
                spec["expression_name"].HasMember("values") && spec["expression_name"]["values"].IsObject());

    const auto& allExpressions = spec["expression_name"]["values"];

    for (auto& entry : allExpressions.GetObject()) {
        const std::string name{entry.name.GetString(), entry.name.GetStringLength()};
        JSDocument document;
        document.Parse<0>(R"([")" + name + R"("])");
        const JSValue* expression = &document;

        // TODO: "interpolate-hcl": https://github.com/mapbox/mapbox-gl-native/issues/8720
        // TODO: "interpolate-lab": https://github.com/mapbox/mapbox-gl-native/issues/8720
        if (name == "interpolate-hcl" || name == "interpolate-lab") {
            if (expression::isExpression(conversion::Convertible(expression))) {
                ASSERT_TRUE(false) << "Expression name" << name
                                   << "is implemented - please update Expression.IsExpression test.";
            }
            continue;
        }

        EXPECT_TRUE(expression::isExpression(conversion::Convertible(expression))) << name;
    }
}

class ExpressionEqualityTest : public ::testing::TestWithParam<std::string> {};

TEST_P(ExpressionEqualityTest, ExpressionEquality) {
    const std::string base = std::string("test/fixtures/expression_equality/") + GetParam();

    std::string error;
    auto parse = [&](std::string filename, std::string& error_) -> std::unique_ptr<expression::Expression> {
        rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator> document;
        document.Parse<0>(util::read_file(filename).c_str());
        assert(!document.HasParseError());
        const JSValue* expression = &document;
        expression::ParsingContext ctx;
        expression::ParseResult parsed = ctx.parseExpression(conversion::Convertible(expression));
        if (!parsed) {
            error_ = ctx.getErrors().size() > 0 ? ctx.getErrors()[0].message : "failed to parse";
        };
        return std::move(*parsed);
    };

    std::unique_ptr<expression::Expression> expression_a1 = parse(base + ".a.json", error);
    ASSERT_TRUE(expression_a1) << GetParam() << ": " << error;

    std::unique_ptr<expression::Expression> expression_a2 = parse(base + ".a.json", error);
    ASSERT_TRUE(expression_a2) << GetParam() << ": " << error;

    std::unique_ptr<expression::Expression> expression_b = parse(base + ".b.json", error);
    ASSERT_TRUE(expression_b) << GetParam() << ": " << error;

    EXPECT_TRUE(*expression_a1 == *expression_a2);
    EXPECT_TRUE(*expression_a1 != *expression_b);

    // Exercise the type-not-equal branches
    using namespace expression;
    EXPECT_FALSE(*expression_b == *((expression_b->getKind() == Kind::Literal) ? dsl::id() : dsl::literal(0.0)));
}

static std::vector<std::string> populateNames() {
    std::vector<std::string> test_inputs;

    const std::string ending{".a.json"};

    const std::filesystem::path style_directory{"test/fixtures/expression_equality"};

    for (const auto& file_entry : std::filesystem::directory_iterator(style_directory)) {
        auto file_entry_path = file_entry.path().string();
        if (!file_entry.path().empty() && file_entry_path.length() >= ending.length() &&
            file_entry_path.ends_with(ending)) {
#if ANDROID
            if (file_entry_path.find("number-format") != std::string::npos) {
                continue;
            }
#endif
            auto file_name = file_entry.path().filename().string();
            test_inputs.push_back(file_name.substr(0, file_name.length() - ending.length()));
        }
    }
    return test_inputs;
}

INSTANTIATE_TEST_SUITE_P(Expression, ExpressionEqualityTest, ::testing::ValuesIn(populateNames()));
