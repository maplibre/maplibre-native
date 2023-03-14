#pragma once

#include <mbgl/util/feature.hpp>

#include <string>
#include <vector>

using namespace mbgl;

Value stripPrecision(const Value& value);
std::vector<std::string> tokenize(std::string str);
bool deepEqual(const Value& a, const Value& b);
bool deepEqual(const std::optional<Value>& a, const std::optional<Value>& b);
