#include "fill_extrusion.hpp"
#include <mapbox/earcut.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <string_view>
#include <vector>

namespace {
template <size_t N>
constexpr mln_plugin_string str(const char (&s)[N]) {
    return {s, N - 1};
}
using Point = std::array<double, 2>;
using Ring = std::vector<Point>;
using Polygon = std::vector<Ring>;
#include "rounded.hpp"
struct Vertex {
    float position[3];
    float normal[3];
    float edge;
};
struct Layout {
    double cornerDistance = 0;
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    std::vector<mln_plugin_segment_v1> segments;
    std::vector<mln_plugin_feature_vertex_range_v1> ranges;
    mln_plugin_vertex_stream_v1 stream{};
    mln_plugin_attribute_binding_v1 attributes[3]{};
    mln_plugin_drawable_descriptor_v1 drawable{};
};

double area(const Ring& ring) {
    double sum = 0;
    for (size_t i = 0, j = ring.size() - 1; i < ring.size(); j = i++)
        sum += (ring[j][0] - ring[i][0]) * (ring[i][1] + ring[j][1]);
    return sum;
}

bool validFeature(const mln_plugin_feature_v1* f) {
    if (!f || f->struct_size < sizeof(*f) || f->geometry_type != MLN_PLUGIN_GEOMETRY_POLYGON ||
        (f->point_count && !f->points) || !f->path_offsets || f->point_count > UINT32_MAX || f->path_offsets[0] != 0 ||
        f->path_offsets[f->path_count] != f->point_count)
        return false;
    for (size_t i = 0; i < f->path_count; ++i)
        if (f->path_offsets[i] > f->path_offsets[i + 1]) return false;
    return true;
}

// Match tile polygon winding conventions: the first non-degenerate ring sets
// the exterior orientation, opposite rings are holes in the preceding exterior.
std::vector<Polygon> polygons(const mln_plugin_feature_v1& f) {
    std::vector<Polygon> result;
    double exterior = 0;
    for (size_t i = 0; i < f.path_count; ++i) {
        Ring ring;
        for (size_t j = f.path_offsets[i]; j < f.path_offsets[i + 1]; ++j)
            ring.push_back({double(f.points[j].x), double(f.points[j].y)});
        if (ring.size() < 3) continue;
        const auto signedArea = area(ring);
        if (!signedArea) continue;
        if (!exterior) exterior = signedArea;
        if (result.empty() || (signedArea < 0) == (exterior < 0)) result.emplace_back();
        result.back().push_back(std::move(ring));
    }
    return result;
}

void reservePrimitive(Layout& l, uint32_t count) {
    if (l.vertices.size() > UINT32_MAX - count || l.indices.size() > UINT32_MAX - 6u)
        throw std::length_error("extrusion geometry exceeds ABI limits");
    if (l.segments.empty() || l.segments.back().vertex_length + count > UINT16_MAX)
        l.segments.push_back(
            {sizeof(mln_plugin_segment_v1), uint32_t(l.vertices.size()), uint32_t(l.indices.size()), 0, 0});
}
Vertex vertex(const Point& p, float top, float nx, float ny, float nz, float edge = 0) {
    const auto q = quantize(p);
    return {{float(q[0]), float(q[1]), top}, {nx, ny, nz}, edge};
}
void emitPolygon(Layout& l, Polygon& poly) {
    // Bound pathological hole counts exactly as the built-in tessellator does.
    if (poly.size() > 501) {
        std::nth_element(poly.begin() + 1, poly.begin() + 501, poly.end(), [](const Ring& a, const Ring& b) {
            return std::abs(area(a)) > std::abs(area(b));
        });
        poly.resize(501);
    }
    if (l.cornerDistance > 0) poly = rounded(std::move(poly), l.cornerDistance);
    std::vector<Point> flat;
    for (const auto& ring : poly) {
        flat.insert(flat.end(), ring.begin(), ring.end());
        uint32_t edgeDistance = 0;
        for (size_t i = 0; i + 1 < ring.size(); ++i) {
            const auto& a = ring[i];
            const auto& b = ring[i + 1];
            const auto qa = quantize(a), qb = quantize(b);
            const float dx = float(qb[0] - qa[0]), dy = float(qb[1] - qa[1]);
            const float length = std::sqrt(dx * dx + dy * dy);
            if (!length) continue;
            const auto distance = static_cast<uint16_t>(std::hypot(b[0] - a[0], b[1] - a[1]));
            const auto nextDistance = (edgeDistance + distance > UINT16_MAX ? 0 : edgeDistance) + distance;
            reservePrimitive(l, 4);
            auto& seg = l.segments.back();
            const auto start = seg.vertex_length;
            const float nx = -dy / length, ny = dx / length;
            l.vertices.insert(l.vertices.end(),
                              {vertex(b, 0, nx, ny, 0, float(edgeDistance)),
                               vertex(b, 1, nx, ny, 0, float(edgeDistance)),
                               vertex(a, 0, nx, ny, 0, float(nextDistance)),
                               vertex(a, 1, nx, ny, 0, float(nextDistance))});
            edgeDistance = nextDistance;
            for (uint32_t n : {0u, 2u, 1u, 1u, 2u, 3u}) l.indices.push_back(uint16_t(start + n));
            seg.vertex_length += 4;
            seg.index_length += 6;
        }
    }
    const auto roof = mapbox::earcut<uint32_t>(poly);
    for (size_t i = 0; i < roof.size(); i += 3) {
        reservePrimitive(l, 3);
        auto& seg = l.segments.back();
        for (size_t j : {i, i + 2, i + 1}) {
            l.indices.push_back(uint16_t(seg.vertex_length++));
            l.vertices.push_back(vertex(flat.at(roof[j]), 1, 0, 0, 1));
        }
        seg.index_length += 3;
    }
}
mln_plugin_status createLayout(const mln_plugin_layout_context_v1* context, void** instance) try {
    if (!context || context->struct_size < sizeof(*context) || !instance ||
        (context->property_count && !context->properties))
        return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    auto layout = std::make_unique<Layout>();
    for (size_t i = 0; i < context->property_count; ++i) {
        const auto& p = context->properties[i];
        if (std::string_view(p.name.data, p.name.size) == "fill-extrusion-rounded-corner-distance")
            layout->cornerDistance = p.value.data.float_value;
    }
    *instance = layout.release();
    return MLN_PLUGIN_STATUS_OK;
} catch (...) {
    return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
}
mln_plugin_status layoutFeature(void* instance, const mln_plugin_feature_v1* f) try {
    if (!instance || !validFeature(f)) return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    auto& l = *static_cast<Layout*>(instance);
    const auto first = uint32_t(l.vertices.size());
    for (auto& poly : polygons(*f)) emitPolygon(l, poly);
    if (l.vertices.size() != first)
        l.ranges.push_back({sizeof(mln_plugin_feature_vertex_range_v1),
                            f->feature_index,
                            1,
                            first,
                            uint32_t(l.vertices.size() - first)});
    return MLN_PLUGIN_STATUS_OK;
} catch (...) {
    return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
}
mln_plugin_status finishLayout(void* instance, mln_plugin_bucket_v1* out) {
    if (!instance || !out || out->struct_size < sizeof(*out)) return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    auto& l = *static_cast<Layout*>(instance);
    l.stream = {sizeof(l.stream),
                0,
                reinterpret_cast<const uint8_t*>(l.vertices.data()),
                l.vertices.size() * sizeof(Vertex),
                uint32_t(l.vertices.size()),
                sizeof(Vertex)};
    l.attributes[0] = {sizeof(mln_plugin_attribute_binding_v1), 0, 0, offsetof(Vertex, position)};
    l.attributes[1] = {sizeof(mln_plugin_attribute_binding_v1), 1, 0, offsetof(Vertex, normal)};
    l.attributes[2] = {sizeof(mln_plugin_attribute_binding_v1), 8, 0, offsetof(Vertex, edge)};
    l.drawable = {sizeof(l.drawable), 1, str("solid"), l.attributes, 3, l.segments.data(), l.segments.size()};
    out->vertex_streams = &l.stream;
    out->vertex_stream_count = l.vertices.empty() ? 0 : 1;
    out->indices = l.indices.data();
    out->index_count = l.indices.size();
    out->drawables = &l.drawable;
    out->drawable_count = l.indices.empty() ? 0 : 1;
    out->feature_vertex_ranges = l.ranges.data();
    out->feature_vertex_range_count = l.ranges.size();
    out->query_radius = 0;
    return MLN_PLUGIN_STATUS_OK;
}
void destroyLayout(void* p) {
    delete static_cast<Layout*>(p);
}

struct alignas(16) Uniforms {
    float matrix[16];
    float color[4];
    float lightColor[3], intensity;
    float lightDirection[3], base;
    float height, opacity, gradient, pad;
    float interpolation[8];
    float patternFrom[4], patternTo[4];
    float pixelUpper[2], pixelLower[2];
    float tileRatio, heightFactor, pixelRatio, fromScale;
    float toScale, fade, textureSize[2];
};
static_assert(sizeof(Uniforms) == 240);
#include "shaders.hpp"
mln_plugin_status updateUniform(const mln_plugin_uniform_context_v1* c, uint32_t id, uint8_t* out, size_t size) {
    if (!c || c->struct_size < sizeof(*c) || id || !out || size != sizeof(Uniforms))
        return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    Uniforms u{};
    std::copy_n(c->tile_matrix, 16, u.matrix);
    std::copy_n(c->light_color, 3, u.lightColor);
    std::copy_n(c->light_direction, 3, u.lightDirection);
    u.intensity = c->light_intensity;
    u.fromScale = c->crossfade_from_scale;
    u.toScale = c->crossfade_to_scale;
    u.fade = c->crossfade_t;
    u.pixelRatio = c->pixel_ratio;
    u.textureSize[0] = c->pattern_texture_size[0];
    u.textureSize[1] = c->pattern_texture_size[1];
    const double tiles = std::pow(2.0, c->tile_z);
    const double nearestTileSize = std::floor(512.0 * std::pow(2.0, std::floor(c->zoom) - c->tile_z));
    const auto x = static_cast<int32_t>(nearestTileSize * (c->tile_x + c->tile_wrap * tiles));
    const auto y = static_cast<int32_t>(nearestTileSize * c->tile_y);
    u.pixelUpper[0] = float(x >> 16);
    u.pixelUpper[1] = float(y >> 16);
    u.pixelLower[0] = float(x & 65535);
    u.pixelLower[1] = float(y & 65535);
    u.tileRatio = float(nearestTileSize / 8192.0);
    u.heightFactor = float(-tiles / 512.0 / 8.0);
    std::memcpy(out, &u, size);
    return MLN_PLUGIN_STATUS_OK;
}
const mln_plugin_value* property(const mln_plugin_property_value_v1* values, size_t count, std::string_view name) {
    for (size_t i = 0; i < count; ++i)
        if (std::string_view(values[i].name.data, values[i].name.size) == name) return &values[i].value;
    return nullptr;
}
mln_plugin_status evaluateLayer(const mln_plugin_property_value_v1* props,
                                size_t count,
                                mln_plugin_layer_evaluation_v1* out) {
    if (!out || out->struct_size < sizeof(*out) || (count && !props)) return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    const auto* opacity = property(props, count, "fill-extrusion-opacity");
    const float alpha = opacity ? opacity->data.float_value : 1;
    bool hasPattern = true; // A data-driven pattern is absent from the camera properties.
    for (size_t i = 0; i < count; ++i) {
        if (std::string_view(props[i].name.data, props[i].name.size) == "fill-extrusion-pattern")
            hasPattern = props[i].explicitly_set;
    }
    out->enabled_passes = alpha <= 0 ? 0 : alpha == 1 && !hasPattern ? 1 : 6;
    if (const auto* t = property(props, count, "fill-extrusion-translate")) out->translation = t->data.float2_value;
    if (const auto* a = property(props, count, "fill-extrusion-translate-anchor"))
        out->translation_anchor_viewport = std::string_view(a->data.string_value.data, a->data.string_value.size) ==
                                           "viewport";
    return MLN_PLUGIN_STATUS_OK;
}
float queryRadius(const mln_plugin_property_statistics_v1*,
                  size_t,
                  const mln_plugin_property_value_v1* props,
                  size_t count) {
    if (const auto* t = property(props, count, "fill-extrusion-translate"))
        return std::hypot(t->data.float2_value.x, t->data.float2_value.y);
    return 0;
}

bool contains(const Point& p, const Ring& ring) {
    bool inside = false;
    for (size_t i = 0, j = ring.size() - 1; i < ring.size(); j = i++) {
        const auto& a = ring[i];
        const auto& b = ring[j];
        if ((a[1] > p[1]) != (b[1] > p[1]) && p[0] < (b[0] - a[0]) * (p[1] - a[1]) / (b[1] - a[1]) + a[0])
            inside = !inside;
    }
    return inside;
}
double side(const Point& a, const Point& b, const Point& p) {
    return (b[0] - a[0]) * (p[1] - a[1]) - (b[1] - a[1]) * (p[0] - a[0]);
}
bool intersects(const Point& a, const Point& b, const Point& c, const Point& d) {
    if (std::max(a[0], b[0]) < std::min(c[0], d[0]) || std::max(c[0], d[0]) < std::min(a[0], b[0]) ||
        std::max(a[1], b[1]) < std::min(c[1], d[1]) || std::max(c[1], d[1]) < std::min(a[1], b[1]))
        return false;
    return side(a, b, c) * side(a, b, d) <= 0 && side(c, d, a) * side(c, d, b) <= 0;
}
uint8_t queryFeature(const mln_plugin_feature_v1* f,
                     const mln_plugin_tile_point_v1* q,
                     size_t count,
                     const mln_plugin_query_context_v1* c,
                     const mln_plugin_property_value_v1* props,
                     size_t propCount) try {
    if (!validFeature(f) || !q || !count || !c || c->struct_size < sizeof(*c)) return 0;
    mln_plugin_layer_evaluation_v1 evaluated{sizeof(evaluated), 0, {}, 0};
    if (evaluateLayer(props, propCount, &evaluated) != MLN_PLUGIN_STATUS_OK) return 0;
    double tx = static_cast<int16_t>(evaluated.translation.x * c->pixels_to_tile_units);
    double ty = static_cast<int16_t>(evaluated.translation.y * c->pixels_to_tile_units);
    if (evaluated.translation_anchor_viewport) {
        const auto angle = -static_cast<float>(c->bearing);
        const auto cosine = std::cos(angle), sine = std::sin(angle);
        const auto x = static_cast<int16_t>(tx * cosine - ty * sine);
        ty = static_cast<int16_t>(tx * sine + ty * cosine);
        tx = x;
    }
    Ring query;
    for (size_t i = 0; i < count; ++i) query.push_back({q[i].x - tx, q[i].y - ty});
    for (const auto& poly : polygons(*f)) {
        for (const auto& p : query) {
            bool in = false;
            for (const auto& ring : poly)
                if (contains(p, ring)) in = !in;
            if (in) return 1;
        }
        if (query.size() > 1)
            for (const auto& ring : poly) {
                for (const auto& p : ring)
                    if (contains(p, query)) return 1;
                for (size_t i = 0; i + 1 < ring.size(); ++i)
                    for (size_t j = 0; j + 1 < query.size(); ++j)
                        if (intersects(ring[i], ring[i + 1], query[j], query[j + 1])) return 1;
            }
    }
    return 0;
} catch (...) {
    return 0;
}

constexpr mln_plugin_value number(float f) {
    mln_plugin_value v{sizeof(v), MLN_PLUGIN_VALUE_FLOAT, {}};
    v.data.float_value = f;
    return v;
}
constexpr mln_plugin_value boolean(bool b) {
    mln_plugin_value v{sizeof(v), MLN_PLUGIN_VALUE_BOOLEAN, {}};
    v.data.boolean_value = b;
    return v;
}
constexpr mln_plugin_value pattern() {
    mln_plugin_value v{sizeof(v), MLN_PLUGIN_VALUE_IMAGE, {}};
    v.data.string_value = str("");
    return v;
}
constexpr mln_plugin_value black() {
    mln_plugin_value v{sizeof(v), MLN_PLUGIN_VALUE_COLOR, {}};
    v.data.color_value = {0, 0, 0, 1};
    return v;
}
constexpr mln_plugin_value translation() {
    mln_plugin_value v{sizeof(v), MLN_PLUGIN_VALUE_FLOAT2, {}};
    return v;
}
constexpr mln_plugin_value anchor() {
    mln_plugin_value v{sizeof(v), MLN_PLUGIN_VALUE_STRING, {}};
    v.data.string_value = str("map");
    return v;
}
constexpr mln_plugin_string anchors[] = {str("map"), str("viewport")};
constexpr mln_plugin_property_descriptor_v1 paint(mln_plugin_string name, mln_plugin_value value, bool data = false) {
    mln_plugin_property_descriptor_v1 p{};
    p.struct_size = sizeof(p);
    p.name = name;
    p.type = value.type;
    p.default_value = value;
    p.expression_capabilities = MLN_PLUGIN_EXPRESSION_CAMERA;
    if (data)
        p.expression_capabilities |= MLN_PLUGIN_EXPRESSION_FEATURE | MLN_PLUGIN_EXPRESSION_COMPOSITE |
                                     MLN_PLUGIN_EXPRESSION_FEATURE_STATE;
    p.supports_transitions = value.type != MLN_PLUGIN_VALUE_BOOLEAN && value.type != MLN_PLUGIN_VALUE_STRING;
    if (value.type == MLN_PLUGIN_VALUE_STRING) {
        p.enum_values = anchors;
        p.enum_value_count = 2;
    }
    return p;
}
constexpr mln_plugin_property_descriptor_v1 roundedProperty() {
    auto p = paint(str("fill-extrusion-rounded-corner-distance"), number(0));
    p.is_layout = 1;
    p.supports_transitions = 0;
    return p;
}
constexpr mln_plugin_property_descriptor_v1 properties[] = {
    roundedProperty(),
    paint(str("fill-extrusion-pattern"), pattern(), true),
    paint(str("fill-extrusion-color"), black(), true),
    paint(str("fill-extrusion-base"), number(0), true),
    paint(str("fill-extrusion-height"), number(0), true),
    paint(str("fill-extrusion-opacity"), number(1)),
    paint(str("fill-extrusion-vertical-gradient"), boolean(true)),
    paint(str("fill-extrusion-translate"), translation()),
    paint(str("fill-extrusion-translate-anchor"), anchor()),
};
constexpr mln_plugin_shader_attribute_v1 attributes[] = {
    {sizeof(mln_plugin_shader_attribute_v1), 0, 0, str("a_pos"), MLN_PLUGIN_VERTEX_FLOAT_X3},
    {sizeof(mln_plugin_shader_attribute_v1), 1, 1, str("a_normal"), MLN_PLUGIN_VERTEX_FLOAT_X3},
    {sizeof(mln_plugin_shader_attribute_v1), 2, 2, str("a_color_min"), MLN_PLUGIN_VERTEX_FLOAT_X4},
    {sizeof(mln_plugin_shader_attribute_v1), 3, 3, str("a_color_max"), MLN_PLUGIN_VERTEX_FLOAT_X4},
    {sizeof(mln_plugin_shader_attribute_v1), 4, 4, str("a_base"), MLN_PLUGIN_VERTEX_FLOAT_X2},
    {sizeof(mln_plugin_shader_attribute_v1), 5, 5, str("a_height"), MLN_PLUGIN_VERTEX_FLOAT_X2},
    {sizeof(mln_plugin_shader_attribute_v1), 6, 6, str("a_opacity"), MLN_PLUGIN_VERTEX_FLOAT_X2},
    {sizeof(mln_plugin_shader_attribute_v1), 7, 7, str("a_gradient"), MLN_PLUGIN_VERTEX_FLOAT_X2},
    {sizeof(mln_plugin_shader_attribute_v1), 8, 8, str("a_edge"), MLN_PLUGIN_VERTEX_FLOAT},
    {sizeof(mln_plugin_shader_attribute_v1), 9, 9, str("a_pattern_from_min"), MLN_PLUGIN_VERTEX_FLOAT_X4},
    {sizeof(mln_plugin_shader_attribute_v1), 10, 10, str("a_pattern_from_max"), MLN_PLUGIN_VERTEX_FLOAT_X4},
    {sizeof(mln_plugin_shader_attribute_v1), 11, 11, str("a_pattern_to_min"), MLN_PLUGIN_VERTEX_FLOAT_X4},
    {sizeof(mln_plugin_shader_attribute_v1), 12, 12, str("a_pattern_to_max"), MLN_PLUGIN_VERTEX_FLOAT_X4},
};
constexpr mln_plugin_shader_property_binding_v1 bindings[] = {
    {sizeof(mln_plugin_shader_property_binding_v1),
     str("fill-extrusion-pattern"),
     MLN_PLUGIN_PROPERTY_ENCODING_IMAGE_FROM,
     0,
     offsetof(Uniforms, patternFrom),
     9,
     10,
     0,
     offsetof(Uniforms, interpolation) + 20},
    {sizeof(mln_plugin_shader_property_binding_v1),
     str("fill-extrusion-pattern"),
     MLN_PLUGIN_PROPERTY_ENCODING_IMAGE_TO,
     0,
     offsetof(Uniforms, patternTo),
     11,
     12,
     0,
     offsetof(Uniforms, interpolation) + 24},
    {sizeof(mln_plugin_shader_property_binding_v1),
     str("fill-extrusion-color"),
     MLN_PLUGIN_PROPERTY_ENCODING_COLOR,
     0,
     offsetof(Uniforms, color),
     2,
     3,
     0,
     offsetof(Uniforms, interpolation)},
    {sizeof(mln_plugin_shader_property_binding_v1),
     str("fill-extrusion-base"),
     MLN_PLUGIN_PROPERTY_ENCODING_FLOAT,
     0,
     offsetof(Uniforms, base),
     4,
     4,
     0,
     offsetof(Uniforms, interpolation) + 4},
    {sizeof(mln_plugin_shader_property_binding_v1),
     str("fill-extrusion-height"),
     MLN_PLUGIN_PROPERTY_ENCODING_FLOAT,
     0,
     offsetof(Uniforms, height),
     5,
     5,
     0,
     offsetof(Uniforms, interpolation) + 8},
    {sizeof(mln_plugin_shader_property_binding_v1),
     str("fill-extrusion-opacity"),
     MLN_PLUGIN_PROPERTY_ENCODING_FLOAT,
     0,
     offsetof(Uniforms, opacity),
     6,
     6,
     0,
     offsetof(Uniforms, interpolation) + 12},
    {sizeof(mln_plugin_shader_property_binding_v1),
     str("fill-extrusion-vertical-gradient"),
     MLN_PLUGIN_PROPERTY_ENCODING_BOOLEAN_FLOAT,
     0,
     offsetof(Uniforms, gradient),
     7,
     7,
     0,
     offsetof(Uniforms, interpolation) + 16},
};
constexpr mln_plugin_shader_source_v1 source{
    sizeof(source), MLN_PLUGIN_BACKEND_VULKAN, str(vertexSource), str(fragmentSource), {}, {}};
constexpr mln_plugin_uniform_block_descriptor_v1 uniform{
    sizeof(uniform),
    0,
    str("ExtrusionUniforms"),
    sizeof(Uniforms),
    MLN_PLUGIN_SHADER_STAGE_VERTEX | MLN_PLUGIN_SHADER_STAGE_FRAGMENT,
    MLN_PLUGIN_UNIFORM_DRAWABLE};
constexpr mln_plugin_shader_descriptor_v1 shader{sizeof(shader),
                                                 str("solid"),
                                                 &source,
                                                 1,
                                                 attributes,
                                                 std::size(attributes),
                                                 &uniform,
                                                 1,
                                                 bindings,
                                                 std::size(bindings),
                                                 1};
constexpr mln_plugin_draw_pass_v1 passes[] = {
    {sizeof(mln_plugin_draw_pass_v1), 1, 1, 1, 1, 0, MLN_PLUGIN_CULL_BACK_CCW},
    {sizeof(mln_plugin_draw_pass_v1), 1, 1, 0, 0, 0, MLN_PLUGIN_CULL_BACK_CCW},
    {sizeof(mln_plugin_draw_pass_v1), 1, 1, 1, 1, 1, MLN_PLUGIN_CULL_BACK_CCW},
};
const mln_plugin_layer_type_v1 layer = [] {
    mln_plugin_layer_type_v1 l{};
    l.struct_size = sizeof(l);
    l.layer_type = str("fill-extrusion");
    l.backend_mask = MLN_PLUGIN_BACKEND_VULKAN;
    l.properties = properties;
    l.property_count = std::size(properties);
    l.geometry_type_mask = MLN_PLUGIN_GEOMETRY_POLYGON;
    l.shaders = &shader;
    l.shader_count = 1;
    l.create_layout = createLayout;
    l.layout_feature = layoutFeature;
    l.finish_layout = finishLayout;
    l.destroy_layout = destroyLayout;
    l.query_feature = queryFeature;
    l.get_query_radius = queryRadius;
    l.update_uniform_block = updateUniform;
    l.replace_builtin = 1;
    l.is_3d = 1;
    l.draw_passes = passes;
    l.draw_pass_count = std::size(passes);
    l.evaluate_layer = evaluateLayer;
    l.enable_near_clipped_matrix = 1;
    return l;
}();
const mln_plugin_descriptor_v1 descriptor{
    sizeof(descriptor), MLN_PLUGIN_ABI_VERSION_1, str("org.maplibre.fill-extrusion"), str("0.1.0"), 1, 1, &layer, 1};
} // namespace
extern "C" mln_plugin_status mln_fill_extrusion_register(mln_plugin_register_function_v1 host,
                                                         char* error,
                                                         size_t capacity) {
    return host ? host(&descriptor, error, capacity) : MLN_PLUGIN_STATUS_NOT_FOUND;
}
