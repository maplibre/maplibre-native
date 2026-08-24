#pragma once

#include <mln/util/feature.hpp>

#include <string>
#include <vector>

using namespace mln;

Value stripPrecision(const Value& value);
std::vector<std::string> tokenize(std::string str);
bool deepEqual(const Value& a, const Value& b);
bool deepEqual(const std::optional<Value>& a, const std::optional<Value>& b);
