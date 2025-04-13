#pragma once

#include <mapbox/feature.hpp>

namespace mapbox {
namespace base {

using Value = feature::value;
using ValueArray = Value::array_type;
using ValueObject = Value::object_type;
using NullValue = feature::null_value_t;

} // namespace base
} // namespace mapbox
