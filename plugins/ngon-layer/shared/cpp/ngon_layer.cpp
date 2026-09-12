#include "ngon_layer.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <string>
#include <string_view>
#include <vector>

#ifndef MLN_NGON_PLUGIN_VERSION
#define MLN_NGON_PLUGIN_VERSION "0.1.0-local"
#endif

namespace {

constexpr uint32_t positionAttribute = 0;
constexpr uint32_t vertexStream = 0;
constexpr uint64_t ngonDrawable = 1;

constexpr mln_plugin_string str(const char* value, size_t size) {
    return {value, size};
}

template <size_t N>
constexpr mln_plugin_string str(const char (&value)[N]) {
    return str(value, N - 1);
}

struct Vertex {
    int16_t position[2];
};

static_assert(offsetof(Vertex, position) == 0);
static_assert(sizeof(Vertex) == 4);

struct Layout {
    uint32_t extent = 8192;
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    std::vector<mln_plugin_segment_v1> segments;
    std::vector<mln_plugin_feature_vertex_range_v1> featureRanges;
    std::array<mln_plugin_vertex_stream_v1, 1> streams{};
    std::array<mln_plugin_attribute_binding_v1, 1> attributes{};
    std::array<mln_plugin_drawable_descriptor_v1, 1> drawables{};
};

void startSegment(Layout& layout) {
    mln_plugin_segment_v1 segment{};
    segment.struct_size = sizeof(segment);
    segment.vertex_offset = static_cast<uint32_t>(layout.vertices.size());
    segment.index_offset = static_cast<uint32_t>(layout.indices.size());
    layout.segments.push_back(segment);
}

mln_plugin_status createLayout(const mln_plugin_layout_context_v1* context, void** instance) try {
    if (!context || context->struct_size < sizeof(*context) || !instance) {
        return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    }
    auto layout = std::unique_ptr<Layout>(new (std::nothrow) Layout());
    if (!layout) return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
    layout->extent = context->extent;
    startSegment(*layout);
    *instance = layout.release();
    return MLN_PLUGIN_STATUS_OK;
} catch (...) {
    return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
}

mln_plugin_status layoutFeature(void* instance, const mln_plugin_feature_v1* feature) try {
    if (!instance || !feature || feature->struct_size < sizeof(*feature) ||
        feature->geometry_type != MLN_PLUGIN_GEOMETRY_POINT || !feature->points) {
        return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    }
    auto& layout = *static_cast<Layout*>(instance);
    const auto firstVertex = static_cast<uint32_t>(layout.vertices.size());

    for (size_t pointIndex = 0; pointIndex < feature->point_count; ++pointIndex) {
        const auto point = feature->points[pointIndex];
        // Half-open ownership avoids duplicate buffered features in adjacent tiles.
        if (point.x < 0 || point.y < 0 || point.x >= static_cast<int>(layout.extent) ||
            point.y >= static_cast<int>(layout.extent) || point.x > 16383 || point.y > 16383)
            continue;
        auto& segment = layout.segments.back();
        if (segment.vertex_length > std::numeric_limits<uint16_t>::max() - 4u) {
            startSegment(layout);
        }
        auto& active = layout.segments.back();
        const uint16_t base = static_cast<uint16_t>(active.vertex_length);
        constexpr int16_t corners[4][2] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
        for (const auto& corner : corners) {
            Vertex vertex{};
            vertex.position[0] = static_cast<int16_t>(point.x * 2 + corner[0]);
            vertex.position[1] = static_cast<int16_t>(point.y * 2 + corner[1]);
            layout.vertices.push_back(vertex);
        }
        const uint16_t quad[] = {base,
                                 static_cast<uint16_t>(base + 1),
                                 static_cast<uint16_t>(base + 2),
                                 base,
                                 static_cast<uint16_t>(base + 2),
                                 static_cast<uint16_t>(base + 3)};
        layout.indices.insert(layout.indices.end(), std::begin(quad), std::end(quad));
        active.vertex_length += 4;
        active.index_length += 6;
        active.feature_index = feature->feature_index;
    }
    if (layout.vertices.size() > firstVertex) {
        layout.featureRanges.push_back({sizeof(mln_plugin_feature_vertex_range_v1),
                                        feature->feature_index,
                                        ngonDrawable,
                                        firstVertex,
                                        static_cast<uint32_t>(layout.vertices.size() - firstVertex)});
    }
    return MLN_PLUGIN_STATUS_OK;
} catch (...) {
    return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
}

mln_plugin_status finishLayout(void* instance, mln_plugin_bucket_v1* output) {
    if (!instance || !output || output->struct_size < sizeof(*output)) {
        return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    }
    auto& layout = *static_cast<Layout*>(instance);
    layout.segments.erase(std::remove_if(layout.segments.begin(),
                                         layout.segments.end(),
                                         [](const auto& segment) { return segment.index_length == 0; }),
                          layout.segments.end());
    layout.streams[0] = {sizeof(mln_plugin_vertex_stream_v1),
                         vertexStream,
                         reinterpret_cast<const uint8_t*>(layout.vertices.data()),
                         layout.vertices.size() * sizeof(Vertex),
                         static_cast<uint32_t>(layout.vertices.size()),
                         sizeof(Vertex)};
    layout.attributes = {{
        {sizeof(mln_plugin_attribute_binding_v1),
         positionAttribute,
         vertexStream,
         offsetof(Vertex, position),
         MLN_PLUGIN_VERTEX_INT16_X2},
    }};
    auto& drawable = layout.drawables[0];
    drawable.struct_size = sizeof(drawable);
    drawable.drawable_key = ngonDrawable;
    drawable.shader_id = str("ngon");
    // Point ownership is half-open; markers may extend across their tile edge.
    drawable.attributes = layout.attributes.data();
    drawable.attribute_count = layout.attributes.size();
    drawable.segments = layout.segments.data();
    drawable.segment_count = layout.segments.size();

    output->vertex_streams = layout.vertices.empty() ? nullptr : layout.streams.data();
    output->vertex_stream_count = layout.vertices.empty() ? 0 : layout.streams.size();
    output->indices = layout.indices.data();
    output->index_count = layout.indices.size();
    output->drawables = layout.indices.empty() ? nullptr : layout.drawables.data();
    output->drawable_count = layout.indices.empty() ? 0 : layout.drawables.size();
    output->query_radius = 0.0f;
    output->feature_vertex_ranges = layout.featureRanges.data();
    output->feature_vertex_range_count = layout.featureRanges.size();
    return MLN_PLUGIN_STATUS_OK;
}

void destroyLayout(void* instance) {
    delete static_cast<Layout*>(instance);
}

struct alignas(16) DrawableUBO {
    float matrix[16];
    float camera[4];
    float view[4];
    float radius, corners, rotate, blur;
    float opacity, stroke_width, stroke_opacity, translate_anchor;
    float color[4], stroke_color[4];
    float translate[2], pitch_alignment, pitch_scale;
    float interpolation[16];
};
static_assert(offsetof(DrawableUBO, radius) == 96);
static_assert(offsetof(DrawableUBO, color) == 128);
static_assert(offsetof(DrawableUBO, translate) == 160);
static_assert(offsetof(DrawableUBO, interpolation) == 176);
static_assert(sizeof(DrawableUBO) == 240);

#include "ngon_shader_sources.hpp"

mln_plugin_status updateUniform(const mln_plugin_uniform_context_v1* context,
                                uint32_t id,
                                uint8_t* output,
                                size_t size) {
    if (!context || context->struct_size < sizeof(*context) || id != 0 || !output || size != sizeof(DrawableUBO)) {
        return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    }
    DrawableUBO value{};
    std::copy_n(context->tile_matrix, 16, value.matrix);
    value.camera[0] = context->pixels_to_gl_units[0];
    value.camera[1] = context->pixels_to_gl_units[1];
    value.camera[2] = context->pixels_to_tile_units;
    value.camera[3] = context->camera_to_center_distance;
    value.view[0] = static_cast<float>(context->bearing);
    value.view[1] = context->pixel_ratio;
    std::memcpy(output, &value, sizeof(value));
    return MLN_PLUGIN_STATUS_OK;
}

constexpr mln_plugin_value number(float n) {
    mln_plugin_value v{};
    v.struct_size = sizeof(v);
    v.type = MLN_PLUGIN_VALUE_FLOAT;
    v.data.float_value = n;
    return v;
}
constexpr mln_plugin_value color() {
    mln_plugin_value v{};
    v.struct_size = sizeof(v);
    v.type = MLN_PLUGIN_VALUE_COLOR;
    v.data.color_value = {0, 0, 0, 1};
    return v;
}
constexpr mln_plugin_value translate() {
    mln_plugin_value v{};
    v.struct_size = sizeof(v);
    v.type = MLN_PLUGIN_VALUE_FLOAT2;
    v.data.float2_value = {0, 0};
    return v;
}
constexpr mln_plugin_value text(mln_plugin_string s) {
    mln_plugin_value v{};
    v.struct_size = sizeof(v);
    v.type = MLN_PLUGIN_VALUE_STRING;
    v.data.string_value = s;
    return v;
}
constexpr mln_plugin_string anchors[] = {str("map"), str("viewport")};
constexpr mln_plugin_property_descriptor_v1 property(mln_plugin_string name,
                                                     mln_plugin_value value,
                                                     float minimum = -std::numeric_limits<float>::infinity(),
                                                     float maximum = std::numeric_limits<float>::infinity()) {
    mln_plugin_property_descriptor_v1 p{};
    p.struct_size = sizeof(p);
    p.name = name;
    p.type = value.type;
    p.default_value = value;
    p.expression_capabilities = MLN_PLUGIN_EXPRESSION_CAMERA | MLN_PLUGIN_EXPRESSION_FEATURE |
                                MLN_PLUGIN_EXPRESSION_COMPOSITE | MLN_PLUGIN_EXPRESSION_FEATURE_STATE;
    p.supports_transitions = value.type != MLN_PLUGIN_VALUE_STRING;
    p.has_minimum = minimum != -std::numeric_limits<float>::infinity();
    p.minimum = p.has_minimum ? minimum : 0;
    p.has_maximum = maximum != std::numeric_limits<float>::infinity();
    p.maximum = p.has_maximum ? maximum : 0;
    if (value.type == MLN_PLUGIN_VALUE_STRING) {
        p.enum_values = anchors;
        p.enum_value_count = 2;
    }
    return p;
}

const mln_plugin_property_descriptor_v1 properties[] = {
    property(str("ngon-radius"), number(5), 0),
    property(str("ngon-corners"), number(5), 3, 360),
    property(str("ngon-rotate"), number(0)),
    property(str("ngon-color"), color()),
    property(str("ngon-blur"), number(0), 0, 1),
    property(str("ngon-opacity"), number(1), 0, 1),
    property(str("ngon-stroke-width"), number(0), 0),
    property(str("ngon-stroke-color"), color()),
    property(str("ngon-stroke-opacity"), number(1), 0, 1),
    property(str("ngon-translate"), translate()),
    property(str("ngon-translate-anchor"), text(str("map"))),
    property(str("ngon-pitch-alignment"), text(str("viewport"))),
    property(str("ngon-pitch-scale"), text(str("map"))),
};

struct Point {
    double x, y;
};
constexpr double pi = 3.14159265358979323846;
double cross(Point a, Point b, Point c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}
bool onSegment(Point a, Point b, Point p) {
    return std::abs(cross(a, b, p)) < 1e-8 && p.x >= std::min(a.x, b.x) - 1e-8 && p.x <= std::max(a.x, b.x) + 1e-8 &&
           p.y >= std::min(a.y, b.y) - 1e-8 && p.y <= std::max(a.y, b.y) + 1e-8;
}
bool intersects(Point a, Point b, Point c, Point d) {
    return ((cross(a, b, c) > 0) != (cross(a, b, d) > 0) && (cross(c, d, a) > 0) != (cross(c, d, b) > 0)) ||
           onSegment(a, b, c) || onSegment(a, b, d) || onSegment(c, d, a) || onSegment(c, d, b);
}
bool contains(const std::vector<Point>& polygon, Point p) {
    if (polygon.size() < 3) return false;
    bool inside = false;
    for (size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
        const auto a = polygon[i], b = polygon[j];
        if (onSegment(a, b, p)) return true;
        if ((a.y > p.y) != (b.y > p.y) && p.x < (b.x - a.x) * (p.y - a.y) / (b.y - a.y) + a.x) inside = !inside;
    }
    return inside;
}
bool overlaps(const std::vector<Point>& a, const std::vector<Point>& b) {
    if (a.empty() || b.empty()) return false;
    if (contains(a, b.front()) || contains(b, a.front())) return true;
    for (size_t i = 0; i < a.size(); ++i)
        for (size_t j = 0; j < b.size(); ++j)
            if (intersects(a[i], a[(i + 1) % a.size()], b[j], b[(j + 1) % b.size()])) return true;
    return false;
}

const mln_plugin_value* valueFor(const mln_plugin_property_value_v1* values, size_t count, std::string_view name) {
    for (size_t i = 0; i < count; ++i) {
        if (std::string_view(values[i].name.data, values[i].name.size) == name) return &values[i].value;
    }
    return nullptr;
}
double scalar(const mln_plugin_property_value_v1* values, size_t count, std::string_view name, double fallback) {
    const auto* value = valueFor(values, count, name);
    return value && value->type == MLN_PLUGIN_VALUE_FLOAT ? value->data.float_value : fallback;
}
bool viewport(const mln_plugin_property_value_v1* values, size_t count, std::string_view name, bool fallback) {
    const auto* value = valueFor(values, count, name);
    if (!value || value->type != MLN_PLUGIN_VALUE_STRING) return fallback;
    const std::string_view anchor(value->data.string_value.data, value->data.string_value.size);
    return anchor == "viewport" ? true : anchor == "map" ? false : fallback;
}
std::array<double, 4> project(Point p, const double* m) {
    return {m[0] * p.x + m[4] * p.y + m[12],
            m[1] * p.x + m[5] * p.y + m[13],
            m[2] * p.x + m[6] * p.y + m[14],
            m[3] * p.x + m[7] * p.y + m[15]};
}
Point screen(const std::array<double, 4>& p, const mln_plugin_query_context_v1& c) {
    return {(p[0] / p[3] + 1) * 0.5 * c.viewport_width, (1 - p[1] / p[3]) * 0.5 * c.viewport_height};
}

uint8_t queryFeature(const mln_plugin_feature_v1* feature,
                     const mln_plugin_tile_point_v1* query,
                     size_t queryCount,
                     const mln_plugin_query_context_v1* context,
                     const mln_plugin_property_value_v1* values,
                     size_t count) try {
    if (!feature || !feature->points || !context || context->struct_size < sizeof(*context) || !query || !queryCount ||
        (count && !values) || context->camera_to_center_distance <= 0)
        return 0;
    const double radius = std::max(0.0, scalar(values, count, "ngon-radius", 5));
    const double stroke = std::max(0.0, scalar(values, count, "ngon-stroke-width", 0));
    if (radius + stroke <= 0) return 0;
    const int corners = static_cast<int>(std::clamp(std::round(scalar(values, count, "ngon-corners", 5)), 3.0, 360.0));
    const double outer = radius + stroke / std::cos(pi / corners);
    const double rotation = scalar(values, count, "ngon-rotate", 0) * pi / 180;
    const bool alignViewport = viewport(values, count, "ngon-pitch-alignment", true);
    const bool scaleViewport = viewport(values, count, "ngon-pitch-scale", false);
    Point offset{};
    if (const auto* t = valueFor(values, count, "ngon-translate"); t && t->type == MLN_PLUGIN_VALUE_FLOAT2)
        offset = {t->data.float2_value.x, t->data.float2_value.y};
    if (viewport(values, count, "ngon-translate-anchor", false)) {
        const double c = std::cos(-context->bearing), s = std::sin(-context->bearing);
        offset = {c * offset.x - s * offset.y, s * offset.x + c * offset.y};
    }
    std::vector<Point> queryScreen;
    for (size_t i = 0; i < queryCount; ++i)
        queryScreen.push_back(
            screen(project({double(query[i].x), double(query[i].y)}, context->tile_matrix), *context));
    for (size_t i = 0; i < feature->point_count; ++i) {
        const Point center = {feature->points[i].x + offset.x * context->pixels_to_tile_units,
                              feature->points[i].y + offset.y * context->pixels_to_tile_units};
        const auto clip = project(center, context->tile_matrix);
        if (clip[3] <= 0) continue;
        const auto centerScreen = screen(clip, *context);
        const double scale = alignViewport ? (scaleViewport ? 1 : context->camera_to_center_distance / clip[3])
                                           : context->pixels_to_tile_units *
                                                 (scaleViewport ? clip[3] / context->camera_to_center_distance : 1);
        std::vector<Point> polygon;
        polygon.reserve(corners);
        for (int n = 0; n < corners; ++n) {
            const double theta = rotation + 2 * pi * n / corners;
            const Point delta = {std::sin(theta) * outer * scale, -std::cos(theta) * outer * scale};
            polygon.push_back(
                alignViewport
                    ? Point{centerScreen.x + delta.x, centerScreen.y + delta.y}
                    : screen(project({center.x + delta.x, center.y + delta.y}, context->tile_matrix), *context));
        }
        if (overlaps(polygon, queryScreen)) return 1;
    }
    return 0;
} catch (...) {
    return 0;
}

float queryRadius(const mln_plugin_property_statistics_v1* stats,
                  size_t count,
                  const mln_plugin_property_value_v1*,
                  size_t) {
    float radius = 5, stroke = 0, tx = 0, ty = 0;
    for (size_t i = 0; i < count; ++i) {
        const auto& s = stats[i];
        const std::string_view name(s.property_name.data, s.property_name.size);
        if (name == "ngon-radius") radius = s.maximum.data.float_value;
        if (name == "ngon-stroke-width") stroke = s.maximum.data.float_value;
        if (name == "ngon-translate") {
            tx = std::max(std::abs(s.minimum.data.float2_value.x), std::abs(s.maximum.data.float2_value.x));
            ty = std::max(std::abs(s.minimum.data.float2_value.y), std::abs(s.maximum.data.float2_value.y));
        }
    }
    return std::max(0.0f, radius) + 2 * std::max(0.0f, stroke) + std::hypot(tx, ty);
}

const mln_plugin_shader_source_v1 shaderSources[] = {
    {sizeof(mln_plugin_shader_source_v1), MLN_PLUGIN_BACKEND_OPENGL, str(openglVertex), str(openglFragment), {}, {}},
    {sizeof(mln_plugin_shader_source_v1), MLN_PLUGIN_BACKEND_VULKAN, str(vulkanVertex), str(vulkanFragment), {}, {}},
    {sizeof(mln_plugin_shader_source_v1),
     MLN_PLUGIN_BACKEND_METAL,
     str(metalSource),
     {},
     str("ngonVertex"),
     str("ngonFragment")},
};
const mln_plugin_uniform_block_descriptor_v1 uniforms[] = {
    {sizeof(mln_plugin_uniform_block_descriptor_v1),
     0,
     str("NgonDrawableUBO"),
     sizeof(DrawableUBO),
     MLN_PLUGIN_SHADER_STAGE_VERTEX},
};
const mln_plugin_shader_descriptor_v1 shader = {
    sizeof(mln_plugin_shader_descriptor_v1),
    str("ngon"),
    shaderSources,
    std::size(shaderSources),
    shaderAttributes,
    std::size(shaderAttributes),
    uniforms,
    std::size(uniforms),
    propertyBindings,
    std::size(propertyBindings),
};
const mln_plugin_layer_type_v1 layerType = [] {
    mln_plugin_layer_type_v1 v{};
    v.struct_size = sizeof(v);
    v.layer_type = str("ngon");
    v.backend_mask = MLN_PLUGIN_BACKEND_OPENGL | MLN_PLUGIN_BACKEND_VULKAN | MLN_PLUGIN_BACKEND_METAL;
    v.properties = properties;
    v.property_count = std::size(properties);
    v.geometry_type_mask = MLN_PLUGIN_GEOMETRY_POINT;
    v.shaders = &shader;
    v.shader_count = 1;
    v.create_layout = createLayout;
    v.layout_feature = layoutFeature;
    v.finish_layout = finishLayout;
    v.destroy_layout = destroyLayout;
    v.query_feature = queryFeature;
    v.get_query_radius = queryRadius;
    v.update_uniform_block = updateUniform;
    return v;
}();
const mln_plugin_descriptor_v1 descriptor = {
    sizeof(mln_plugin_descriptor_v1),
    MLN_PLUGIN_ABI_VERSION_1,
    str("org.maplibre.ngon-layer"),
    str(MLN_NGON_PLUGIN_VERSION),
    MLN_PLUGIN_ABI_VERSION_1,
    MLN_PLUGIN_ABI_VERSION_1,
    &layerType,
    1,
};
} // namespace

extern "C" mln_plugin_status mln_ngon_layer_register(mln_plugin_register_function_v1 registerPlugin,
                                                     char* error,
                                                     size_t capacity) {
    if (!registerPlugin) return MLN_PLUGIN_STATUS_NOT_FOUND;
    return registerPlugin(&descriptor, error, capacity);
}
