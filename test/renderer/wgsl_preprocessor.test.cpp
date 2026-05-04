#include <mbgl/test/util.hpp>
#include <mbgl/shaders/webgpu/wgsl_preprocessor.hpp>

using namespace mbgl;
using namespace mbgl::webgpu::detail;

TEST(ShaderGroupPreprocessor, isDirective_ValidDirectives) {
    EXPECT_TRUE(isDirective("#ifdef FEATURE", "ifdef"));
    EXPECT_TRUE(isDirective("#ifndef FEATURE", "ifndef"));
    EXPECT_TRUE(isDirective("#else", "else"));
    EXPECT_TRUE(isDirective("#endif", "endif"));

    // With leading whitespace
    EXPECT_TRUE(isDirective("  #ifdef FEATURE", "ifdef"));
    EXPECT_TRUE(isDirective("\t#ifdef FEATURE", "ifdef"));
    EXPECT_TRUE(isDirective("   \t  #ifdef FEATURE", "ifdef"));

    // With whitespace after #
    EXPECT_TRUE(isDirective("#  ifdef FEATURE", "ifdef"));
    EXPECT_TRUE(isDirective("#\tifdef FEATURE", "ifdef"));
    EXPECT_TRUE(isDirective("#   ifdef FEATURE", "ifdef"));
}

TEST(ShaderGroupPreprocessor, isDirective_InvalidDirectives) {
    EXPECT_FALSE(isDirective("ifdef FEATURE", "ifdef"));     // Missing #
    EXPECT_FALSE(isDirective("#ifdef FEATURE", "ifndef"));   // Wrong directive
    EXPECT_FALSE(isDirective("#ifdefine FEATURE", "ifdef")); // Similar but not exact
    EXPECT_FALSE(isDirective("// #ifdef FEATURE", "ifdef")); // Commented out
    EXPECT_FALSE(isDirective("", "ifdef"));                  // Empty line
    EXPECT_FALSE(isDirective("   ", "ifdef"));               // Whitespace only
}

TEST(ShaderGroupPreprocessor, getDirectiveArgument_ValidArguments) {
    EXPECT_EQ(getDirectiveArgument("#ifdef FEATURE"), "FEATURE");
    EXPECT_EQ(getDirectiveArgument("#ifndef DEBUG_MODE"), "DEBUG_MODE");
    EXPECT_EQ(getDirectiveArgument("#ifdef  FEATURE"), "FEATURE");  // Extra spaces
    EXPECT_EQ(getDirectiveArgument("  #ifdef FEATURE"), "FEATURE"); // Leading whitespace
    EXPECT_EQ(getDirectiveArgument("#ifdef\tFEATURE"), "FEATURE");  // Tab separator
}

TEST(ShaderGroupPreprocessor, getDirectiveArgument_InvalidOrEmpty) {
    EXPECT_EQ(getDirectiveArgument(""), "");               // Empty line
    EXPECT_EQ(getDirectiveArgument("#ifdef"), "");         // No argument
    EXPECT_EQ(getDirectiveArgument("#else"), "");          // No argument expected
    EXPECT_EQ(getDirectiveArgument("#endif"), "");         // No argument expected
    EXPECT_EQ(getDirectiveArgument("no hash symbol"), ""); // Invalid format
}

TEST(ShaderGroupPreprocessor, getDirectiveArgument_MultipleWords) {
    // Should only return the first word
    EXPECT_EQ(getDirectiveArgument("#ifdef FEATURE_A FEATURE_B"), "FEATURE_A");
    EXPECT_EQ(getDirectiveArgument("#ifdef FEATURE\nmore text"), "FEATURE");
}

TEST(ShaderGroupPreprocessor, preprocessWGSL_SimpleIfdef) {
    std::unordered_map<std::string, bool> defines;
    defines["ENABLED"] = true;

    std::string source = R"(
line1
#ifdef ENABLED
line2
#endif
line3
)";

    std::string result = preprocessWGSL(source, defines);
    EXPECT_NE(result.find("line1"), std::string::npos);
    EXPECT_NE(result.find("line2"), std::string::npos);
    EXPECT_NE(result.find("line3"), std::string::npos);
}

TEST(ShaderGroupPreprocessor, preprocessWGSL_IfdefDisabled) {
    std::unordered_map<std::string, bool> defines;
    // ENABLED is not defined

    std::string source = R"(
line1
#ifdef ENABLED
line2
#endif
line3
)";

    std::string result = preprocessWGSL(source, defines);
    EXPECT_NE(result.find("line1"), std::string::npos);
    EXPECT_EQ(result.find("line2"), std::string::npos); // Should be excluded
    EXPECT_NE(result.find("line3"), std::string::npos);
}

TEST(ShaderGroupPreprocessor, preprocessWGSL_Ifndef) {
    std::unordered_map<std::string, bool> defines;
    // DISABLED is not defined

    std::string source = R"(
line1
#ifndef DISABLED
line2
#endif
line3
)";

    std::string result = preprocessWGSL(source, defines);
    EXPECT_NE(result.find("line1"), std::string::npos);
    EXPECT_NE(result.find("line2"), std::string::npos); // Should be included
    EXPECT_NE(result.find("line3"), std::string::npos);
}

TEST(ShaderGroupPreprocessor, preprocessWGSL_IfndefWithDefine) {
    std::unordered_map<std::string, bool> defines;
    defines["DISABLED"] = true;

    std::string source = R"(
line1
#ifndef DISABLED
line2
#endif
line3
)";

    std::string result = preprocessWGSL(source, defines);
    EXPECT_NE(result.find("line1"), std::string::npos);
    EXPECT_EQ(result.find("line2"), std::string::npos); // Should be excluded
    EXPECT_NE(result.find("line3"), std::string::npos);
}

TEST(ShaderGroupPreprocessor, preprocessWGSL_Else) {
    std::unordered_map<std::string, bool> defines;
    defines["ENABLED"] = true;

    std::string source = R"(
#ifdef ENABLED
line1
#else
line2
#endif
)";

    std::string result = preprocessWGSL(source, defines);
    EXPECT_NE(result.find("line1"), std::string::npos);
    EXPECT_EQ(result.find("line2"), std::string::npos);
}

TEST(ShaderGroupPreprocessor, preprocessWGSL_ElseInverted) {
    std::unordered_map<std::string, bool> defines;
    // ENABLED is not defined

    std::string source = R"(
#ifdef ENABLED
line1
#else
line2
#endif
)";

    std::string result = preprocessWGSL(source, defines);
    EXPECT_EQ(result.find("line1"), std::string::npos);
    EXPECT_NE(result.find("line2"), std::string::npos);
}

TEST(ShaderGroupPreprocessor, preprocessWGSL_NestedConditionals) {
    std::unordered_map<std::string, bool> defines;
    defines["OUTER"] = true;
    defines["INNER"] = true;

    std::string source = R"(
line1
#ifdef OUTER
line2
#ifdef INNER
line3
#endif
line4
#endif
line5
)";

    std::string result = preprocessWGSL(source, defines);
    EXPECT_NE(result.find("line1"), std::string::npos);
    EXPECT_NE(result.find("line2"), std::string::npos);
    EXPECT_NE(result.find("line3"), std::string::npos);
    EXPECT_NE(result.find("line4"), std::string::npos);
    EXPECT_NE(result.find("line5"), std::string::npos);
}

TEST(ShaderGroupPreprocessor, preprocessWGSL_NestedConditionalsPartiallyEnabled) {
    std::unordered_map<std::string, bool> defines;
    defines["OUTER"] = true;
    // INNER is not defined

    std::string source = R"(
line1
#ifdef OUTER
line2
#ifdef INNER
line3
#endif
line4
#endif
line5
)";

    std::string result = preprocessWGSL(source, defines);
    EXPECT_NE(result.find("line1"), std::string::npos);
    EXPECT_NE(result.find("line2"), std::string::npos);
    EXPECT_EQ(result.find("line3"), std::string::npos); // Inner block excluded
    EXPECT_NE(result.find("line4"), std::string::npos);
    EXPECT_NE(result.find("line5"), std::string::npos);
}

TEST(ShaderGroupPreprocessor, preprocessWGSL_ComplexNesting) {
    std::unordered_map<std::string, bool> defines;
    defines["A"] = true;
    defines["B"] = true;

    std::string source = R"(
#ifdef A
insideA1
#ifdef B
insideB
#else
notB
#endif
insideA2
#endif
outside
)";

    std::string result = preprocessWGSL(source, defines);
    EXPECT_NE(result.find("insideA1"), std::string::npos);
    EXPECT_NE(result.find("insideB"), std::string::npos);
    EXPECT_EQ(result.find("notB"), std::string::npos);
    EXPECT_NE(result.find("insideA2"), std::string::npos);
    EXPECT_NE(result.find("outside"), std::string::npos);
}

TEST(ShaderGroupPreprocessor, preprocessWGSL_EmptySource) {
    std::unordered_map<std::string, bool> defines;

    std::string source = "";
    std::string result = preprocessWGSL(source, defines);
    EXPECT_EQ(result, "");
}

TEST(ShaderGroupPreprocessor, preprocessWGSL_NoConditionals) {
    std::unordered_map<std::string, bool> defines;

    std::string source = R"(
line1
line2
line3
)";

    std::string result = preprocessWGSL(source, defines);
    EXPECT_NE(result.find("line1"), std::string::npos);
    EXPECT_NE(result.find("line2"), std::string::npos);
    EXPECT_NE(result.find("line3"), std::string::npos);
}

TEST(ShaderGroupPreprocessor, preprocessWGSL_PreservesNewlines) {
    std::unordered_map<std::string, bool> defines;
    defines["ENABLED"] = true;

    std::string source = "line1\n#ifdef ENABLED\nline2\n#endif\nline3";
    std::string result = preprocessWGSL(source, defines);

    // Count newlines - should have 3 (after line1, line2, line3)
    size_t newlineCount = std::count(result.begin(), result.end(), '\n');
    EXPECT_EQ(newlineCount, 3);
}

TEST(ShaderGroupPreprocessor, preprocessWGSL_MismatchedEndif) {
    std::unordered_map<std::string, bool> defines;

    // Extra endif should be ignored gracefully
    std::string source = R"(
line1
#endif
line2
)";

    std::string result = preprocessWGSL(source, defines);
    EXPECT_NE(result.find("line1"), std::string::npos);
    EXPECT_NE(result.find("line2"), std::string::npos);
}
