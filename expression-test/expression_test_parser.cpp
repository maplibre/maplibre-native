#include "expression_test_parser.hpp"

#if defined(WIN32) && defined(GetObject)
#undef GetObject
#endif

#include <mbgl/util/io.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/variant.hpp>
#include <mbgl/style/conversion/function.hpp>
#include <mbgl/style/rapidjson_conversion.hpp>
#include <mbgl/style/expression/parsing_context.hpp>

#include <mapbox/geojson/rapidjson.hpp>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <args.hxx>

#include <regex>

using namespace mbgl;
using namespace mbgl::style;
using namespace mbgl::style::conversion;
using namespace std::literals;

namespace {

void writeJSON(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, const Value& value) {
    value.match([&](const NullValue&) { writer.Null(); },
                [&](bool b) { writer.Bool(b); },
                [&](uint64_t u) { writer.Uint64(u); },
                [&](int64_t i) { writer.Int64(i); },
                [&](double d) { d == std::floor(d) ? writer.Int64(static_cast<int64_t>(d)) : writer.Double(d); },
                [&](const std::string& s) { writer.String(s); },
                [&](const std::vector<Value>& arr) {
                    writer.StartArray();
                    for (const auto& item : arr) {
                        writeJSON(writer, item);
                    }
                    writer.EndArray();
                },
                [&](const std::unordered_map<std::string, Value>& obj) {
                    writer.StartObject();
                    for (const auto& entry : obj) {
                        writer.Key(entry.first.c_str());
                        writeJSON(writer, entry.second);
                    }
                    writer.EndObject();
                });
}

using ErrorMessage = std::string;
using JSONReply = variant<JSDocument, ErrorMessage>;
JSONReply readJson(const std::filesystem::path& jsonPath) {
    auto maybeJSON = util::readFile(jsonPath);
    if (!maybeJSON) {
        return {"Unable to open file "s + jsonPath.string()};
    }

    JSDocument document;
    document.Parse<rapidjson::kParseFullPrecisionFlag>(*maybeJSON);
    if (document.HasParseError()) {
        return {formatJSONParseError(document)};
    }

    return {std::move(document)};
}

std::string toString(const JSValue& value) {
    assert(value.IsString());
    return {value.GetString(), value.GetStringLength()};
}

std::optional<Value> toValue(const JSValue& jsvalue) {
    if (jsvalue.IsNull()) {
        return Value{};
    }

    if (jsvalue.IsArray()) {
        std::vector<Value> values;
        values.reserve(jsvalue.GetArray().Size());
        for (const auto& v : jsvalue.GetArray()) {
            if (auto value = toValue(v)) {
                values.emplace_back(std::move(*value));
            }
        }
        return {std::move(values)};
    }

    if (jsvalue.IsObject()) {
        std::unordered_map<std::string, Value> value_map;
        for (const auto& pair : jsvalue.GetObject()) {
            if (auto value = toValue(pair.value)) {
                value_map.emplace(toString(pair.name), std::move(*value));
            }
        }
        return {std::move(value_map)};
    }

    if (!jsvalue.IsArray() && !jsvalue.IsObject()) {
        return toValue(Convertible(&jsvalue));
    }

    return std::nullopt;
}

style::expression::type::Type stringToType(const std::string& type) {
    using namespace style::expression;
    if (type == "string"s || type == "number-format"s) {
        return type::String;
    } else if (type == "number"s) {
        return type::Number;
    } else if (type == "boolean"s) {
        return type::Boolean;
    } else if (type == "object"s) {
        return type::Object;
    } else if (type == "color"s) {
        return type::Color;
    } else if (type == "value"s) {
        return type::Value;
    } else if (type == "formatted"s) {
        return type::Formatted;
    } else if (type == "resolvedImage"s) {
        return type::Image;
    }

    // Should not reach.
    assert(false);
    return type::Null;
}

std::optional<style::expression::type::Type> toExpressionType(const PropertySpec& spec) {
    using namespace style::expression;
    if (spec.type == "array") {
        type::Type itemType = spec.value.empty() ? type::Value : stringToType(spec.value);
        if (spec.length) {
            return {type::Array(itemType, spec.length)};
        }
        return {type::Array(itemType)};
    }

    if (spec.type == "enum") {
        return {type::String};
    }

    return spec.type.empty() ? std::nullopt : std::optional<type::Type>{stringToType(spec.type)};
}

void parseCompiled(const JSValue& compiledValue, TestData& data) {
    const auto& compiled = compiledValue.GetObject();
    assert(compiled.HasMember("result"));
    assert(compiled["result"].IsString());
    const std::string& result = toString(compiled["result"]);
    data.expected.compiled.success = result == "success";

    if (compiled.HasMember("isFeatureConstant")) {
        assert(compiled["isFeatureConstant"].IsBool());
        data.expected.compiled.isFeatureConstant = compiled["isFeatureConstant"].GetBool();
    }

    if (compiled.HasMember("isZoomConstant")) {
        assert(compiled["isZoomConstant"].IsBool());
        data.expected.compiled.isZoomConstant = compiled["isZoomConstant"].GetBool();
    }

    if (compiled.HasMember("type")) {
        assert(compiled["type"].IsString());
        data.expected.compiled.serializedType = toString(compiled["type"]);
    }

    if (compiled.HasMember("errors")) {
        assert(compiled["errors"].IsArray());
        for (const auto& errorVal : compiled["errors"].GetArray()) {
            assert(errorVal.IsObject());
            const auto& errorObject = errorVal.GetObject();
            assert(errorObject.HasMember("key"));
            assert(errorObject.HasMember("error"));

            std::unordered_map<std::string, Value> errorMap;
            errorMap.emplace("key"s, Value{toString(errorObject["key"])});
            errorMap.emplace("error"s, Value{toString(errorObject["error"])});
            data.expected.compiled.errors.emplace_back(Value{std::move(errorMap)});
        }
    }
}

void parseExpected(const JSValue& expectedValue, TestData& data) {
    assert(expectedValue.IsObject());
    const auto& expected = expectedValue.GetObject();
    assert(expected.HasMember("compiled"));
    parseCompiled(expected["compiled"], data);

    // set outputs
    if (expected.HasMember("outputs")) {
        data.expected.outputs = toValue(expected["outputs"]);
    }

    // set serialized
    if (expected.HasMember("serialized")) {
        data.expected.serialized = toValue(expected["serialized"]);
    }
}

void parsePropertySpec(const JSValue& value, TestData& data) {
    const auto& propertySpec = value.GetObject();
    PropertySpec spec;

    if (propertySpec.HasMember("type")) {
        assert(propertySpec["type"].IsString());
        spec.type = toString(propertySpec["type"]);
    }

    if (propertySpec.HasMember("value")) {
        assert(propertySpec["value"].IsString());
        spec.value = toString(propertySpec["value"]);
    }

    if (propertySpec.HasMember("length")) {
        assert(propertySpec["length"].IsNumber());
        spec.length = static_cast<size_t>(propertySpec["length"].GetDouble());
    }

    if (propertySpec.HasMember("property-type")) {
        assert(propertySpec["property-type"].IsString());
        spec.isDataDriven = true;
    }

    if (propertySpec.HasMember("expression")) {
        assert(propertySpec["expression"].IsObject());
        spec.expression = toValue(propertySpec["expression"]);
    }

    data.spec = std::move(spec);
}

bool parseInputs(const JSValue& inputsValue, TestData& data) {
    assert(inputsValue.IsArray());
    for (const auto& input : inputsValue.GetArray()) {
        assert(input.IsArray());
        assert(input.Size() == 2);
        assert(input[0].IsObject());
        assert(input[1].IsObject());

        // Parse evaluation context, zoom.
        std::optional<float> zoom;
        const auto& evaluationContext = input[0].GetObject();
        if (evaluationContext.HasMember("zoom")) {
            assert(evaluationContext["zoom"].IsNumber());
            zoom = static_cast<float>(evaluationContext["zoom"].GetDouble());
        }

        // Parse heatmap density
        std::optional<double> heatmapDensity;
        if (evaluationContext.HasMember("heatmapDensity")) {
            assert(evaluationContext["heatmapDensity"].IsNumber());
            heatmapDensity = evaluationContext["heatmapDensity"].GetDouble();
        }

        // Parse canonicalID
        std::optional<CanonicalTileID> canonical;
        if (evaluationContext.HasMember("canonicalID")) {
            const auto& canonicalIDObject = evaluationContext["canonicalID"];
            assert(canonicalIDObject.IsObject());
            assert(canonicalIDObject.HasMember("z") && canonicalIDObject["z"].IsNumber());
            assert(canonicalIDObject.HasMember("x") && canonicalIDObject["x"].IsNumber());
            assert(canonicalIDObject.HasMember("y") && canonicalIDObject["y"].IsNumber());
            canonical = CanonicalTileID(
                canonicalIDObject["z"].GetUint(), canonicalIDObject["x"].GetUint(), canonicalIDObject["y"].GetUint());
        }

        // Parse availableImages
        std::set<std::string> availableImages;
        if (evaluationContext.HasMember("availableImages")) {
            assert(evaluationContext["availableImages"].IsArray());
            for (const auto& image : evaluationContext["availableImages"].GetArray()) {
                assert(image.IsString());
                availableImages.emplace(toString(image));
            }
        }

        // Parse feature properties
        Feature feature(mapbox::geometry::point<double>(0.0, 0.0));
        const auto& featureObject = input[1].GetObject();
        if (featureObject.HasMember("properties")) {
            assert(featureObject["properties"].IsObject());
            feature.properties = mapbox::geojson::convert<PropertyMap>(featureObject["properties"]);
        }

        if (featureObject.HasMember("geometry")) {
            assert(featureObject["geometry"].IsObject());
            feature.geometry = mapbox::geojson::convert<mapbox::geometry::geometry<double>>(featureObject["geometry"]);
        }

        if (featureObject.HasMember("id")) {
            assert(featureObject["id"].IsNumber() || featureObject["id"].IsString());
            feature.id = mapbox::geojson::convert<mapbox::feature::identifier>(featureObject["id"]);
        }

        data.inputs.emplace_back(std::move(zoom),
                                 std::move(heatmapDensity),
                                 std::move(canonical),
                                 std::move(availableImages),
                                 std::move(feature));
    }
    return true;
}

} // namespace

std::tuple<std::filesystem::path, std::vector<std::filesystem::path>, bool, uint32_t> parseArguments(int argc,
                                                                                                     char** argv) {
    args::ArgumentParser argumentParser("Mapbox GL Expression Test Runner");

    args::HelpFlag helpFlag(argumentParser, "help", "Display this help menu", {'h', "help"});
    args::Flag shuffleFlag(argumentParser, "shuffle", "Toggle shuffling the tests order", {'s', "shuffle"});
    args::ValueFlag<uint32_t> seedValue(argumentParser, "seed", "Shuffle seed (default: random)", {"seed"});
    args::PositionalList<std::string> testNameValues(argumentParser, "URL", "Test name(s)");
    args::ValueFlag<std::string> testFilterValue(argumentParser, "filter", "Test filter regex", {'f', "filter"});

    try {
        argumentParser.ParseCLI(argc, argv);
    } catch (const args::Help&) {
        std::ostringstream stream;
        stream << argumentParser;
        Log::Info(Event::General, stream.str());
        exit(0);
    } catch (const args::ParseError& e) {
        std::ostringstream stream;
        stream << argumentParser;
        Log::Info(Event::General, stream.str());
        Log::Error(Event::General, e.what());
        exit(1);
    } catch (const args::ValidationError& e) {
        std::ostringstream stream;
        stream << argumentParser;
        Log::Info(Event::General, stream.str());
        Log::Error(Event::General, e.what());
        exit(2);
    }

    std::filesystem::path rootPath{std::string(TEST_RUNNER_ROOT_PATH).append("/metrics/integration/expression-tests")};
    if (!std::filesystem::exists(rootPath)) {
        Log::Error(Event::General, "Test path '" + rootPath.string() + "' does not exist.");
        exit(3);
    }

    std::vector<std::filesystem::path> paths;
    for (const auto& testName : args::get(testNameValues)) {
        paths.emplace_back(rootPath.string() + "/" + testName);
    }

    if (paths.empty()) {
        paths.emplace_back(rootPath);
    }

    auto testFilter = testFilterValue ? args::get(testFilterValue) : std::string{};
    // Recursively traverse through the test paths and collect test directories containing "test.json".
    std::vector<std::filesystem::path> testPaths;
    testPaths.reserve(paths.size());
    for (const auto& path : paths) {
        if (!std::filesystem::exists(path)) {
            Log::Warning(Event::General, "Provided test folder '" + path.string() + "' does not exist.");
            continue;
        }

        for (auto& testPath : std::filesystem::recursive_directory_iterator(path)) {
            if (!testFilter.empty() && !std::regex_search(testPath.path().string(), std::regex(testFilter))) {
                continue;
            }
            if (testPath.path().filename() == "test.json") {
                testPaths.emplace_back(testPath.path());
            }
        }
    }

    return Arguments{std::move(rootPath),
                     std::move(testPaths),
                     shuffleFlag ? args::get(shuffleFlag) : false,
                     seedValue ? args::get(seedValue) : 1u};
}

Ignores parseExpressionIgnores() {
    Ignores ignores;
    const auto mainIgnoresPath =
        std::filesystem::path(TEST_RUNNER_ROOT_PATH).append("metrics/ignores/platform-all.json");
    auto maybeIgnores = readJson(mainIgnoresPath);
    if (!maybeIgnores.is<JSDocument>()) { // NOLINT
        return {};
    }

    for (const auto& property : maybeIgnores.get<JSDocument>().GetObject()) {
        std::string id{toString(property.name)};
        // Keep only expression-test ignores
        if (id.rfind("expression-tests", 0) != 0) {
            continue;
        }
        std::string reason{toString(property.value)};
        ignores.emplace_back(std::move(id), std::move(reason));
    }

    return ignores;
}

std::optional<TestData> parseTestData(const std::filesystem::path& path) {
    try {
        TestData data;
        auto maybeJson = readJson(path.string());
        if (!maybeJson.is<JSDocument>()) { // NOLINT
            mbgl::Log::Error(mbgl::Event::General, "Cannot parse test '" + path.string() + "'.");
            return std::nullopt;
        }

        data.document = std::move(maybeJson.get<JSDocument>());

        // Check that mandatory test data members are present.
        if (!data.document.HasMember("expression") || !data.document.HasMember("expected")) {
            Log::Error(Event::General, "Test fixture '" + path.string() + "' does not contain required data.");
            return std::nullopt;
        }

        // Parse propertySpec
        if (data.document.HasMember("propertySpec")) {
            assert(data.document["propertySpec"].IsObject());
            parsePropertySpec(data.document["propertySpec"], data);
        }

        // Parse expected
        parseExpected(data.document["expected"], data);

        // Parse inputs
        if (data.document.HasMember("inputs") && !parseInputs(data.document["inputs"], data)) {
            Log::Error(Event::General, std::string("Can't convert inputs value for '") + path.string() + "'");
            return std::nullopt;
        }

        return {std::move(data)};
    } catch (const std::exception& ex) {
        Log::Error(Event::General, std::string("Cannot load test data from '" + path.string() + "': " + ex.what()));
        return std::nullopt;
    }
}

std::string toJSON(const Value& value, unsigned indent, bool singleLine) {
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    if (singleLine) {
        writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
    }
    writer.SetIndent(' ', indent);
    writeJSON(writer, value);
    return buffer.GetString();
}

JSDocument toDocument(const Value& value) {
    JSDocument document;
    document.Parse<rapidjson::kParseFullPrecisionFlag>(toJSON(value));
    return document;
}

Value toValue(const Compiled& compiled) {
    std::unordered_map<std::string, Value> compiledObject;
    compiledObject.emplace("result", compiled.success ? "success"s : "error"s);
    if (compiled.success) {
        compiledObject.emplace("isFeatureConstant", compiled.isFeatureConstant);
        compiledObject.emplace("isZoomConstant", compiled.isZoomConstant);
        compiledObject.emplace("type", compiled.serializedType);
    } else {
        compiledObject.emplace("errors", compiled.errors);
    }
    return {std::move(compiledObject)};
}

// Serializes native expression types to JS format.
// Color and Formatted are exceptions.
// TODO: harmonize serialized format to remove this conversion.
std::optional<Value> toValue(const expression::Value& exprValue) {
    return exprValue.match(
        [](const Color& c) -> std::optional<Value> {
            std::vector<Value> color{
                static_cast<double>(c.r), static_cast<double>(c.g), static_cast<double>(c.b), static_cast<double>(c.a)};
            return {Value{std::move(color)}};
        },
        [](const expression::Formatted& formatted) -> std::optional<Value> { return {formatted.toObject()}; },
        [](const std::vector<expression::Value>& values) -> std::optional<Value> {
            std::vector<Value> mbglValues;
            for (const auto& value : values) {
                if (auto converted = expression::fromExpressionValue<Value>(value)) {
                    mbglValues.emplace_back(std::move(*converted));
                }
            }
            return {Value{std::move(mbglValues)}};
        },
        [](const std::unordered_map<std::string, expression::Value>& valueMap) -> std::optional<Value> {
            std::unordered_map<std::string, Value> mbglValueMap;
            for (const auto& pair : valueMap) {
                if (auto converted = expression::fromExpressionValue<Value>(pair.second)) {
                    mbglValueMap.emplace(pair.first, std::move(*converted));
                }
            }
            return {Value{std::move(mbglValueMap)}};
        },
        [](const auto& v) { return expression::fromExpressionValue<Value>(v); });
}

std::unique_ptr<style::expression::Expression> parseExpression(const JSValue& value,
                                                               std::optional<PropertySpec>& spec,
                                                               TestResult& result) {
    std::optional<style::expression::type::Type> expected = spec ? toExpressionType(*spec) : std::nullopt;
    expression::ParsingContext ctx = expected ? expression::ParsingContext(*expected) : expression::ParsingContext();
    Convertible convertible(&value);
    expression::ParseResult parsed;
    if (value.IsObject() && !value.IsArray() && expected) {
        Error error;
        parsed = convertFunctionToExpression(*expected, convertible, error, false /*convert tokens*/);
        if (!parsed) {
            // TODO: should the error message be checked for function conversion?
        }
    } else {
        parsed = ctx.parseLayerPropertyExpression(convertible);
        if (!parsed) {
            for (const auto& parsingError : ctx.getErrors()) {
                std::unordered_map<std::string, Value> errorMap;
                errorMap.emplace("key"s, Value{parsingError.key});
                errorMap.emplace("error"s, Value{parsingError.message});
                result.compiled.errors.emplace_back(Value{std::move(errorMap)});
            }
        }
    }

    result.expression = toValue(value);
    result.compiled.success = bool(parsed);
    if (parsed) {
        result.compiled.isFeatureConstant = expression::isFeatureConstant(**parsed);
        result.compiled.isZoomConstant = expression::isZoomConstant(**parsed);
        result.compiled.serializedType = style::expression::type::toString((*parsed)->getType());
        result.serialized = (*parsed)->serialize();
        return std::move(*parsed);
    }

    return nullptr;
}

std::unique_ptr<style::expression::Expression> parseExpression(const std::optional<Value>& value,
                                                               std::optional<PropertySpec>& spec,
                                                               TestResult& result) {
    assert(value);
    auto document = toDocument(*value);
    assert(!document.HasParseError());
    return parseExpression(document, spec, result);
}
