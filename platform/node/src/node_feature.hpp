#pragma once

#include <mln/util/feature.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wshadow"
#include <nan.h>
#pragma GCC diagnostic pop

namespace node_mbgl {

v8::Local<v8::Value> toJS(const mln::Value&);
v8::Local<v8::Object> toJS(const mln::Feature&);
v8::Local<v8::Object> toJS(const mln::Feature::geometry_type&);
v8::Local<v8::Object> toJS(const mln::PropertyMap&);

} // namespace node_mbgl
