#include <mbgl/test/util.hpp>
#include <mbgl/test/fixture_log_observer.hpp>

#include <mbgl/style/expression/dsl.hpp>
#include <mbgl/style/parser.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/util/enum.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/util/tileset.hpp>

#include <rapidjson/document.h>

#include <iostream>
#include <fstream>
#include <set>

#if defined(WIN32)
#include <windows.h>
#ifdef GetObject
#undef GetObject
#endif
#else
#include <dirent.h>
#endif

using namespace mbgl;

using Message = std::pair<uint32_t, std::string>;
using Messages = std::vector<Message>;

class StyleParserTest : public ::testing::TestWithParam<std::string> {};

TEST_P(StyleParserTest, ParseStyle) {
    const std::string base = std::string("test/fixtures/style_parser/") + GetParam();

    using namespace std::string_literals;
    SCOPED_TRACE("Loading: "s + base);

    FixtureLog log;

    rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator> infoDoc;
    infoDoc.Parse<0>(util::read_file(base + ".info.json").c_str());
    ASSERT_FALSE(infoDoc.HasParseError());
    ASSERT_TRUE(infoDoc.IsObject());

    style::Parser parser;
    if (auto error = parser.parse(util::read_file(base + ".style.json"))) {
        Log::Error(Event::ParseStyle, "Failed to parse style: " + util::toString(error));
    }

    ASSERT_TRUE(infoDoc.IsObject());
    for (const auto& property : infoDoc.GetObject()) {
        const std::string name{property.name.GetString(), property.name.GetStringLength()};
        const JSValue& value = property.value;
        ASSERT_EQ(true, value.IsObject());

        if (value.HasMember("log")) {
            const JSValue& js_log = value["log"];
            ASSERT_EQ(true, js_log.IsArray());
            for (auto& js_entry : js_log.GetArray()) {
                ASSERT_EQ(true, js_entry.IsArray());
                ASSERT_GE(4u, js_entry.Size());

                const uint32_t count = js_entry[rapidjson::SizeType(0)].GetUint();
                const FixtureLogObserver::LogMessage message{
                    *Enum<EventSeverity>::toEnum(js_entry[rapidjson::SizeType(1)].GetString()),
                    *Enum<Event>::toEnum(js_entry[rapidjson::SizeType(2)].GetString()),
                    int64_t(-1),
                    js_entry[rapidjson::SizeType(3)].GetString()};

#if defined(WIN32)
                Sleep(10);
#endif

                SCOPED_TRACE("Checking: "s + message.msg);

                const auto observedCount = log.count(message);
                EXPECT_EQ(count, observedCount) << "Message: " << message << std::endl;
            }
        }

        const auto& unchecked = log.unchecked();
        if (unchecked.size()) {
            std::cerr << "Unchecked Log Messages (" << base << "/" << name << "): " << std::endl << unchecked;
        }

        ASSERT_EQ(0u, unchecked.size());
    }
}

static void populateNames(std::vector<std::string>& names) {
    const std::string ending = ".info.json";

    std::string style_directory = "test/fixtures/style_parser";

    auto testName = [&](const std::string& name) {
        if (name.length() >= ending.length() && name.ends_with(ending)) {
            names.push_back(name.substr(0, name.length() - ending.length()));
        }
    };

#if defined(WIN32)
    style_directory += "/*";
    WIN32_FIND_DATAA ffd;
    HANDLE hFind = FindFirstFileA(style_directory.c_str(), &ffd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            const std::string name = ffd.cFileName;
            testName(name);
        } while (FindNextFileA(hFind, &ffd) != 0);
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(style_directory.c_str());
    if (dir != nullptr) {
        for (dirent* dp = nullptr; (dp = readdir(dir)) != nullptr;) {
            const std::string name = dp->d_name;
            testName(name);
        }
        closedir(dir);
    }
#endif
}

INSTANTIATE_TEST_SUITE_P(StyleParser, StyleParserTest, ::testing::ValuesIn([] {
                             std::vector<std::string> names;
                             populateNames(names);
                             EXPECT_GT(names.size(), 0u);
                             return names;
                         }()));

TEST(StyleParser, SpriteAsString) {
    style::Parser parser;
    parser.parse(R"({
        "version": 8,
        "sprite": "https://example.com/default/markers"
    })");
    auto result = &parser.sprites;
    ASSERT_EQ(1, result->size());
    ASSERT_EQ("https://example.com/default/markers", result->at(0).spriteURL);
    ASSERT_EQ("default", result->at(0).id);
}

TEST(StyleParser, SpriteAsArrayEmpty) {
    style::Parser parser;
    parser.parse(R"({
        "version": 8,
        "sprite": []
    })");
    auto result = &parser.sprites;
    ASSERT_EQ(0, result->size());
}

TEST(StyleParser, SpriteAsArraySingle) {
    style::Parser parser;
    parser.parse(R"({
        "version": 8,
        "sprite": [{
            "id": "default",
            "url": "https://example.com/default/markers"
        }]
    })");
    auto result = &parser.sprites;
    ASSERT_EQ(1, result->size());
    ASSERT_EQ("https://example.com/default/markers", result->at(0).spriteURL);
    ASSERT_EQ("default", result->at(0).id);
}

TEST(StyleParser, SpriteAsArrayMultiple) {
    style::Parser parser;
    parser.parse(R"({
        "version": 8,
        "sprite": [{
            "id": "default",
            "url": "https://example.com/default/markers"
        },{
            "id": "hiking",
            "url": "https://example.com/hiking/markers"
        }]
    })");
    auto result = &parser.sprites;
    ASSERT_EQ(2, result->size());
    ASSERT_EQ("https://example.com/default/markers", result->at(0).spriteURL);
    ASSERT_EQ("default", result->at(0).id);
    ASSERT_EQ("https://example.com/hiking/markers", result->at(1).spriteURL);
    ASSERT_EQ("hiking", result->at(1).id);
}

TEST(StyleParser, FontStacks) {
    style::Parser parser;
    parser.parse(util::read_file("test/fixtures/style_parser/font_stacks.json"));
    std::set<mbgl::FontStack> expected = {
        {"a"},
        {"a", "b"},
        {"a", "b", "c"},
    };
    std::set<mbgl::FontStack> result = parser.fontStacks();
    ASSERT_EQ(expected, result);
}

TEST(StyleParser, FontStacksNoTextField) {
    style::Parser parser;
    parser.parse(R"({
        "version": 8,
        "layers": [{
            "id": "symbol",
            "type": "symbol",
            "source": "vector",
            "layout": {
                "text-font": ["a"]
            }
        }]
    })");
    auto result = parser.fontStacks();
    ASSERT_EQ(0u, result.size());
}

TEST(StyleParser, FontStacksCaseExpression) {
    style::Parser parser;
    parser.parse(R"({
        "version": 8,
        "layers": [{
            "id": "symbol",
            "type": "symbol",
            "source": "vector",
            "layout": {
                "text-field": "a",
                "text-font": ["case", ["==", "a", ["string", ["get", "text-font"]]], ["literal", ["Arial"]], ["literal", ["Helvetica"]]]
            }
        }]
    })");
    std::set<mbgl::FontStack> expected;
    expected.insert(FontStack({"Arial"}));
    expected.insert(FontStack({"Helvetica"}));
    std::set<mbgl::FontStack> result = parser.fontStacks();
    ASSERT_EQ(expected, result);
}

TEST(StyleParser, FontStacksMatchExpression) {
    style::Parser parser;
    parser.parse(R"({
        "version": 8,
        "layers": [{
            "id": "symbol",
            "type": "symbol",
            "source": "vector",
            "layout": {
                "text-field": "a",
                "text-font": ["match", ["get", "text-font"], "a", ["literal", ["Arial"]], ["literal", ["Helvetica"]]]
            }
        }]
    })");
    std::set<mbgl::FontStack> expected;
    expected.insert(FontStack({"Arial"}));
    expected.insert(FontStack({"Helvetica"}));
    std::set<mbgl::FontStack> result = parser.fontStacks();
    ASSERT_EQ(expected, result);
}

TEST(StyleParser, FontStacksStepExpression) {
    style::Parser parser;
    parser.parse(R"({
        "version": 8,
        "layers": [{
            "id": "symbol",
            "type": "symbol",
            "source": "vector",
            "layout": {
                "text-field": "a",
                "text-font": ["array", "string", ["step", ["get", "text-font"], ["literal", ["Arial"]], 0, ["literal", ["Helvetica"]]]]
            }
        }]
    })");
    std::set<mbgl::FontStack> expected;
    expected.insert(FontStack({"Arial"}));
    expected.insert(FontStack({"Helvetica"}));
    std::set<mbgl::FontStack> result = parser.fontStacks();
    ASSERT_EQ(expected, result);
}

TEST(StyleParser, FontStacksGetExpression) {
    // Invalid style, but not currently validated.
    style::Parser parser;
    parser.parse(R"({
        "version": 8,
        "layers": [{
            "id": "symbol",
            "type": "symbol",
            "source": "vector",
            "layout": {
                "text-field": "a",
                "text-font": ["array", "string", ["get", "text-font"]]
            }
        }]
    })");
    auto result = parser.fontStacks();
    ASSERT_EQ(0u, result.size());
}

TEST(StyleParser, ZoomCurve) {
    using namespace mbgl::style;
    using namespace mbgl::style::expression;
    using namespace mbgl::style::expression::dsl;

    const auto zoomInterp = []() {
        return interpolate(linear(), zoom(), 0.0, literal(0.0), 0.0, literal(0.0));
    };

    auto expr1 = interpolate(linear(), literal(0.0), 0.0, zoomInterp(), 0.0, zoomInterp());
    ASSERT_TRUE(expr1);
    ASSERT_TRUE(findZoomCurveChecked(*expr1).is<std::nullptr_t>());

    auto expr2 = interpolate(linear(), zoom(), 0.0, literal(0.0), 0.0, zoomInterp());
    ASSERT_TRUE(expr2);
    ASSERT_TRUE(findZoomCurveChecked(*expr2).is<std::nullptr_t>());
}
