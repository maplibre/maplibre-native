#pragma once

#include <mbgl/style/expression/expression.hpp>
#include <mbgl/util/feature.hpp>
#include <mbgl/util/rapidjson.hpp>

#include <set>
#include <string>
#include <vector>
#include <filesystem>

using namespace mbgl;

struct Input {
    Input(std::optional<float> zoom_,
          std::optional<double> heatmapDensity_,
          std::optional<CanonicalTileID> canonical_,
          std::set<std::string> availableImages_,
          Feature feature_)
        : zoom(std::move(zoom_)),
          heatmapDensity(std::move(heatmapDensity_)),
          canonical(std::move(canonical_)),
          availableImages(std::move(availableImages_)),
          feature(std::move(feature_)) {}
    std::optional<float> zoom;
    std::optional<double> heatmapDensity;
    std::optional<CanonicalTileID> canonical;
    std::set<std::string> availableImages;
    Feature feature;
};

struct Compiled {
    bool operator==(const Compiled& other) const {
        bool typeEqual = success == other.success && isFeatureConstant == other.isFeatureConstant &&
                         isZoomConstant == other.isZoomConstant && serializedType == other.serializedType &&
                         errors == other.errors;
        return typeEqual;
    }

    bool success = false;
    bool isFeatureConstant = false;
    bool isZoomConstant = false;
    std::string serializedType;
    std::vector<Value> errors;
};

struct TestResult {
    Compiled compiled;
    std::optional<Value> expression;
    std::optional<Value> outputs;
    std::optional<Value> serialized;
};

struct PropertySpec {
    std::string type;
    std::string value;
    std::size_t length = 0;
    bool isDataDriven = false;
    std::optional<Value> expression;
};

class TestData {
public:
    std::vector<Input> inputs;
    TestResult expected;
    TestResult result;
    TestResult recompiled;
    std::optional<PropertySpec> spec;
    JSDocument document;
};

struct Ignore {
    Ignore(std::string id_, std::string reason_)
        : id(std::move(id_)),
          reason(std::move(reason_)) {}

    std::string id;
    std::string reason;
};

using Arguments = std::tuple<std::filesystem::path, std::vector<std::filesystem::path>, bool, uint32_t>;
Arguments parseArguments(int argc, char** argv);

using Ignores = std::vector<Ignore>;
Ignores parseExpressionIgnores();
std::optional<TestData> parseTestData(const std::filesystem::path&);

std::string toJSON(const Value& value, unsigned indent = 0, bool singleLine = false);
JSDocument toDocument(const Value&);
Value toValue(const Compiled&);
std::optional<Value> toValue(const style::expression::Value&);

std::unique_ptr<style::expression::Expression> parseExpression(const JSValue&,
                                                               std::optional<PropertySpec>&,
                                                               TestResult&);
std::unique_ptr<style::expression::Expression> parseExpression(const std::optional<Value>&,
                                                               std::optional<PropertySpec>&,
                                                               TestResult&);