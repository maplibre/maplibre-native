#include <array>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4267)
#endif

#include <mapbox/cheap_ruler.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/math/angles.hpp>
#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/layers/render_location_indicator_layer.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/style/layers/location_indicator_layer.hpp>
#include <mbgl/style/layers/location_indicator_layer_impl.hpp>
#include <mbgl/style/layers/location_indicator_layer_properties.hpp>
#include <mbgl/util/mat4.hpp>
#include <mbgl/util/tile_cover.hpp>
#include <mbgl/gfx/context.hpp>

#include <mapbox/eternal.hpp>
#include <mbgl/renderer/image_manager.hpp>

#if MLN_RENDER_BACKEND_OPENGL

#include <mbgl/platform/gl_functions.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/gl/renderable_resource.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/uniform.hpp>
#include <mbgl/gl/types.hpp>

#endif

#ifdef MLN_DRAWABLE_LOCATION_INDICATOR

#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/shaders/location_indicator_ubo.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/renderer/layers/location_indicator_layer_tweaker.hpp>

#endif

#include <numbers>

using namespace mbgl::platform;
using namespace std::numbers;

namespace mbgl {

struct LocationIndicatorRenderParameters {
    LocationIndicatorRenderParameters() = default;
    explicit LocationIndicatorRenderParameters(const TransformParameters& tp)
        : state(&tp.state) {}
    LocationIndicatorRenderParameters(const LocationIndicatorRenderParameters& o) = default;
    LocationIndicatorRenderParameters& operator=(const LocationIndicatorRenderParameters& o) = default;

    double width = 0.0;
    double height = 0.0;
    double latitude = 0.0;
    double longitude = 0.0;
    double zoom = 0.0;
    double bearing = 0.0;
    double pitch = 0.0;
    std::array<double, 16> projectionMatrix;
    const TransformState* state = nullptr;
    ImageManager* imageManager = nullptr;
    // some testing defaults, for before it gets updated via props
    double puckBearing = 0.0;
    LatLng puckPosition = {0, 0};
    double errorRadiusMeters;
    mbgl::Color errorRadiusColor{0, 0, 0, 0};
    mbgl::Color errorRadiusBorderColor{0, 0, 0, 0};
    float puckScale = 0;
    float puckHatScale = 0;
    float puckShadowScale = 0;
    float puckLayersDisplacement = 0;
    float perspectiveCompensation = 0;
    std::string puckImagePath;
    std::string puckShadowImagePath;
    std::string puckHatImagePath;
};

class RenderLocationIndicatorImpl {
public:
    struct vec2 {
        float x = 0.0f;
        float y = 0.0f;

        vec2(float x_, float y_)
            : x(x_),
              y(y_) {}
        vec2() = default;
        explicit vec2(const Point<double>& p)
            : x(static_cast<float>(p.x)),
              y(static_cast<float>(p.y)) {}
        vec2(const vec2& o) = default;
        vec2(vec2&& o) = default;
        vec2& operator=(vec2&& o) = default;
        vec2& operator=(const vec2& o) = default;
        float length() const { return std::sqrt(x * x + y * y); }
        vec2 normalized() const {
            const float size = length();
            return {x / size, y / size};
        }
        void normalize() { *this = normalized(); }
        vec2 mirrored(const vec2& mirror) const {
            float k = dot(mirror) / mirror.length();
            return 2.0 * k * mirror - (*this);
        }
        float dot(const vec2& v2) const { return x * v2.x + y * v2.y; }
        vec2 rotated(float degrees) const {
            const float cs = std::cos(util::deg2radf(degrees));
            const float sn = std::sin(util::deg2radf(degrees));
            return vec2{x * cs + y * sn, x * sn + y * cs}.normalized();
        }
        float bearing() const {
            const vec2 norm = normalized();

            // From theta to bearing
            return util::rad2degf(
                util::wrap<float>((pi_v<float> / 2) - std::atan2(-norm.y, norm.x), 0.0f, pi_v<float> * 2));
        }
        Point<double> toPoint() const { return {x, y}; }

        friend vec2 operator-(const vec2& v) { return {-v.x, -v.y}; }
        friend vec2 operator*(double a, const vec2& v) { return {float(v.x * a), float(v.y * a)}; }
        friend vec2 operator+(const vec2& v1, const vec2& v2) { return {v1.x + v2.x, v1.y + v2.y}; }
        friend vec2 operator-(const vec2& v1, const vec2& v2) { return {v1.x - v2.x, v1.y - v2.y}; }
    };

#ifndef MLN_DRAWABLE_LOCATION_INDICATOR
    struct Shader {
        virtual ~Shader() { release(); }
        void release() {
            if (!program) return;
            MBGL_CHECK_ERROR(glDetachShader(program, vertexShader));
            MBGL_CHECK_ERROR(glDetachShader(program, fragmentShader));
            MBGL_CHECK_ERROR(glDeleteShader(vertexShader));
            MBGL_CHECK_ERROR(glDeleteShader(fragmentShader));
            MBGL_CHECK_ERROR(glDeleteProgram(program));
            program = vertexShader = fragmentShader = 0;
        }
        void initialize(const GLchar* const& vsSource, const GLchar* const& fsSource) {
            if (program) return;
            program = MBGL_CHECK_ERROR(glCreateProgram());
            vertexShader = MBGL_CHECK_ERROR(glCreateShader(GL_VERTEX_SHADER));
            fragmentShader = MBGL_CHECK_ERROR(glCreateShader(GL_FRAGMENT_SHADER));
            MBGL_CHECK_ERROR(glShaderSource(vertexShader, 1, &vsSource, nullptr));
            MBGL_CHECK_ERROR(glCompileShader(vertexShader));
            MBGL_CHECK_ERROR(glAttachShader(program, vertexShader));
            MBGL_CHECK_ERROR(glShaderSource(fragmentShader, 1, &fsSource, nullptr));
            MBGL_CHECK_ERROR(glCompileShader(fragmentShader));
            MBGL_CHECK_ERROR(glAttachShader(program, fragmentShader));
            MBGL_CHECK_ERROR(glLinkProgram(program));
            pullLocations();
        }
        virtual void bind() { MBGL_CHECK_ERROR(glUseProgram(program)); }
        void detach() { MBGL_CHECK_ERROR(glUseProgram(0)); }
        virtual void pullLocations() {};

        GLuint program = 0;
        GLuint vertexShader = 0;
        GLuint fragmentShader = 0;
    };

    struct SimpleShader : public Shader {
        // Note that custom layers need to draw geometry with a z value of 1 to
        // take advantage of depth-based fragment culling.
        const GLchar* vertexShaderSource = R"MBGL_SHADER(
#ifdef GL_ES
precision highp float;
#endif

attribute vec2 a_pos;
uniform mat4 u_matrix;
void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);
}
)MBGL_SHADER";

        const GLchar* fragmentShaderSource = R"MBGL_SHADER(
#ifdef GL_ES
precision highp float;
#endif

uniform vec4 u_color;
void main() {
    gl_FragColor = u_color;
}
)MBGL_SHADER";

        void initialize() { Shader::initialize(SimpleShader::vertexShaderSource, SimpleShader::fragmentShaderSource); }

        void pullLocations() override {
            a_pos = MBGL_CHECK_ERROR(glGetAttribLocation(program, "a_pos"));
            u_color = MBGL_CHECK_ERROR(glGetUniformLocation(program, "u_color"));
            u_matrix = MBGL_CHECK_ERROR(glGetUniformLocation(program, "u_matrix"));
        }
        void bind() override {
            SimpleShader::initialize();
            Shader::bind();
        }

        GLuint a_pos = 0;
        GLuint u_color = 0;
        GLuint u_matrix = 0;
    };

    struct TexturedShader : public Shader {
        const GLchar* vertexShaderSource = R"MBGL_SHADER(
#ifdef GL_ES
precision highp float;
#endif

attribute vec2 a_pos;
attribute vec2 a_texCoord;
uniform mat4 u_matrix;
varying vec2 v_texCoord;
void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);
    v_texCoord = a_texCoord;
}
)MBGL_SHADER";

        const GLchar* fragmentShaderSource = R"MBGL_SHADER(
#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D u_image;
varying vec2 v_texCoord;
void main() {
    vec4 color = texture2D(u_image, v_texCoord);
    gl_FragColor = color;
}
)MBGL_SHADER";

        void initialize() { Shader::initialize(vertexShaderSource, fragmentShaderSource); }

        void pullLocations() override {
            a_pos = MBGL_CHECK_ERROR(glGetAttribLocation(program, "a_pos"));
            a_texCoord = MBGL_CHECK_ERROR(glGetAttribLocation(program, "a_texCoord"));
            u_image = MBGL_CHECK_ERROR(glGetUniformLocation(program, "u_image"));
            u_matrix = MBGL_CHECK_ERROR(glGetUniformLocation(program, "u_matrix"));
        }
        void bind() override {
            TexturedShader::initialize();
            Shader::bind();
        }

        GLuint a_pos = 0;
        GLuint a_texCoord = 0;
        GLuint u_image = 0;
        GLuint u_matrix = 0;
    };

    struct Buffer {
        virtual ~Buffer() { release(); }
        void release() {
            if (!bufferId) return;
            MBGL_CHECK_ERROR(glDeleteBuffers(1, &bufferId));
            bufferId = 0;
        }
        void initialize() {
            if (!bufferId) MBGL_CHECK_ERROR(glGenBuffers(1, &bufferId));
        }
        void bind(const GLenum target = GL_ARRAY_BUFFER) {
            initialize();
            MBGL_CHECK_ERROR(glBindBuffer(target, bufferId));
        }
        void detach(const GLenum target = GL_ARRAY_BUFFER) { MBGL_CHECK_ERROR(glBindBuffer(target, 0)); }
        template <typename T, std::size_t N>
        void upload(const std::array<T, N>& data) {
            bind();
            MBGL_CHECK_ERROR(glBufferData(GL_ARRAY_BUFFER, N * sizeof(T), data.data(), GL_STATIC_DRAW));
            size = static_cast<unsigned int>(N * sizeof(T));
            elements = N;
        }
        template <typename T>
        void upload(const std::vector<T>& data) {
            bind();
            MBGL_CHECK_ERROR(glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW));
            size = data.size() * sizeof(T);
            elements = data.size();
        }

        GLuint bufferId = 0;
        unsigned int size = 0;
        unsigned int elements = 0;
    };

public:
    struct Texture {
        ~Texture() { release(); }
        void release() {
            MBGL_CHECK_ERROR(glDeleteTextures(1, &texId));
            texId = 0;
            image = nullptr;
        }
        /*
            Assign can be called any time. Conversely, upload must be called
           with a bound gl context.
        */
        void assign(const Immutable<style::Image::Impl>* img) {
            imageDirty = true;
            image = (img) ? &(img->get()->image) : nullptr;
            width = height = 0;
            pixelRatio = 1.0f;
            if (image) {
                sharedImage = *img; // keep reference until uploaded
                width = image->size.width;
                height = image->size.height;
                pixelRatio = img->get()->pixelRatio;
            } else {
                sharedImage = std::nullopt;
            }
        }

        void upload() {
            if (!imageDirty) return;
            imageDirty = false;
            initialize();

            MBGL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, texId));
            if (!sharedImage || !image || !image->valid()) {
                MBGL_CHECK_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
            } else {
                MBGL_CHECK_ERROR(glTexImage2D(GL_TEXTURE_2D,
                                              0,
                                              GL_RGBA,
                                              image->size.width,
                                              image->size.height,
                                              0,
                                              GL_RGBA,
                                              GL_UNSIGNED_BYTE,
                                              image->data.get()));
                MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
                MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
                MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
                MBGL_CHECK_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
                if (RenderLocationIndicatorImpl::anisotropicFilteringAvailable)
                    MBGL_CHECK_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16));
            }
            detach();
            sharedImage = std::nullopt;
        }
        void initialize() {
            if (texId != 0) return;
            MBGL_CHECK_ERROR(glGenTextures(1, &texId));
        }
        void bind(int textureUnit = -1) {
            initialize();
            if (!image && !imageDirty) return;

            upload();
            if (textureUnit >= 0) MBGL_CHECK_ERROR(glActiveTexture(GL_TEXTURE0 + textureUnit));
            MBGL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, texId));
        }
        void detach() { MBGL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, 0)); }
        bool isValid() { return imageDirty || image; }
        GLuint texId = 0;
        const mbgl::PremultipliedImage* image = nullptr;
        std::optional<Immutable<style::Image::Impl>> sharedImage;
        bool imageDirty = false;
        size_t width = 0;
        size_t height = 0;
        float pixelRatio = 1.0f;
    };

    static bool hasAnisotropicFiltering() {
        const auto* extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) { // glGetString(GL_EXTENSIONS) is deprecated in OpenGL
                                    // Desktop 3.0+. But OpenGL 3.0+ has anisotropic filtering.
            return true;
        } else {
            if (strstr(extensions, "GL_EXT_texture_filter_anisotropic") != nullptr) return true;
        }
        return false;
    }
    void initialize() {
        // Check if anisotropic filtering is available
        if (initialized) return;
        initialized = true;
        if (hasAnisotropicFiltering()) anisotropicFilteringAvailable = true;
        simpleShader.initialize();
        texturedShader.initialize();
        texCoords = {{{0.0f, 1.0f},
                      {0.0f, 0.0f},
                      {1.0f, 0.0f},
                      {1.0f, 1.0f}}}; // Quads will be drawn as triangle fans. so bl, tl, tr, br
        texCoordsBuffer.upload(texCoords);
    }

    void render(const mbgl::LocationIndicatorRenderParameters& params) {
        initialize();
        drawRadius(params);
        drawShadow();
        drawPuck();
        drawHat();
    }

#endif

#ifdef MLN_DRAWABLE_LOCATION_INDICATOR
    struct TextureInfo {
        std::shared_ptr<gfx::Texture2D> texture;
        std::optional<Immutable<style::Image::Impl>> image;
        std::string id = "";
        float pixelRatio = 1.0f;
        size_t width = 0;
        size_t height = 0;
        bool dirty = false;

        void assign(const Immutable<style::Image::Impl>& img) {
            reset();

            image = img;

            id = img->id;
            pixelRatio = img->pixelRatio;

            width = img->image.size.width;
            height = img->image.size.height;
        }

        void reset() {
            texture.reset();
            image.reset();
            pixelRatio = 1.0f;
            width = 0;
            height = 0;
            dirty = true;
        }
    };

    bool setTextureFromImageID(const std::string& imagePath,
                               TextureInfo& textureInfo,
                               const mbgl::LocationIndicatorRenderParameters& params) {
        bool updated = false;
        const Immutable<style::Image::Impl>* sharedImage = nullptr;

        if (!imagePath.empty() && params.imageManager) {
            sharedImage = params.imageManager->getSharedImage(imagePath);
        }

        if (sharedImage) {
            if (!textureInfo.texture || textureInfo.id != sharedImage->get()->id) {
                textureInfo.assign(*sharedImage);
                updated = true;
            }
        } else if (textureInfo.image) {
            textureInfo.reset();
            updated = true;
        }

        return updated;
    }
#endif

public:
    RenderLocationIndicatorImpl(std::string sourceLayer)
        : ruler(0, mapbox::cheap_ruler::CheapRuler::Meters),
          feature(std::make_shared<mbgl::Feature>()),
          featureEnvelope(std::make_shared<mapbox::geometry::polygon<int64_t>>()) {
        feature->sourceLayer = std::move(sourceLayer);
    }

    void release() {
#ifndef MLN_DRAWABLE_LOCATION_INDICATOR
        if (!simpleShader.program) return;
        for (const auto& t : textures) t.second->release();
        buffer.release();
        circleBuffer.release();
        puckBuffer.release();
        hatBuffer.release();
        texCoordsBuffer.release();
        simpleShader.release();
        texturedShader.release();
#else
        shadowDrawableInfo.reset();
        puckDrawableInfo.reset();
        hatDrawableInfo.reset();
#endif
    }

    void updatePuckGeometry(const mbgl::LocationIndicatorRenderParameters& params) {
        if (params.projectionMatrix != oldParams.projectionMatrix) positionChanged = true;
        if (params.puckPosition != oldParams.puckPosition) {
            positionChanged = true;
            ruler = mapbox::cheap_ruler::CheapRuler(params.puckPosition.latitude(),
                                                    mapbox::cheap_ruler::CheapRuler::Meters);
        } else if (params.puckBearing != oldParams.puckBearing ||
                   params.puckLayersDisplacement != oldParams.puckLayersDisplacement ||
                   params.perspectiveCompensation != oldParams.perspectiveCompensation ||
                   params.puckScale != oldParams.puckScale || params.puckHatScale != oldParams.puckHatScale ||
                   params.puckShadowScale != oldParams.puckShadowScale)
            bearingChanged = true; // changes puck geometry but not necessarily the location
        if (params.errorRadiusMeters != oldParams.errorRadiusMeters) radiusChanged = true;
#ifndef MLN_DRAWABLE_LOCATION_INDICATOR
        bearingChanged |= setTextureFromImageID(params.puckImagePath, texPuck, params);
        bearingChanged |= setTextureFromImageID(params.puckShadowImagePath, texShadow, params);
        bearingChanged |= setTextureFromImageID(params.puckHatImagePath, texPuckHat, params);
#else
        bearingChanged |= setTextureFromImageID(params.puckShadowImagePath, shadowDrawableInfo.textureInfo, params);
        bearingChanged |= setTextureFromImageID(params.puckImagePath, puckDrawableInfo.textureInfo, params);
        bearingChanged |= setTextureFromImageID(params.puckHatImagePath, hatDrawableInfo.textureInfo, params);
#endif

        projectionCircle = params.projectionMatrix;
        const Point<double> positionMercator = project(params.puckPosition, *params.state);
        matrix::identity(translation);
        matrix::translate(translation, translation, positionMercator.x, positionMercator.y, 0.0);
        matrix::multiply(projectionCircle, projectionCircle, translation);

        if (positionChanged) {
            updateRadius(params);
            updatePuck(params);
            positionChanged = false;
        } else {
            if (radiusChanged) {
                updateRadius(params);
            }
            if (bearingChanged) {
                updatePuck(params);
            }
        }
        oldParams = params;
        dirtyFeature = true;
    }

    void updateFeature() {
        if (!dirtyFeature) return;
        dirtyFeature = false;
        featureEnvelope->clear();
#ifndef MLN_DRAWABLE_LOCATION_INDICATOR
        if (!texPuck || !texPuck->isValid()) return;
#else
        if (!puckDrawableInfo.textureInfo.texture) return;

        auto& puckGeometry = puckDrawableInfo.geometry;
#endif

        feature->geometry = mapbox::geometry::point<double>{oldParams.puckPosition.latitude(),
                                                            oldParams.puckPosition.longitude()};
        mapbox::geometry::linear_ring<int64_t> border;
        for (const auto& v : puckGeometry) {
            vec4 p{{v.x, v.y, 0, 1}};
            matrix::transformMat4(p, p, translation);
            border.push_back(Point<int64_t>{int64_t(p[0]), int64_t(p[1])});
        }
        border.push_back(border.front());
        featureEnvelope->push_back(border);
    }

protected:
    static ScreenCoordinate latLngToScreenCoordinate(const LatLng& p, const TransformState& s) {
        LatLng unwrappedLatLng = p.wrapped();
        unwrappedLatLng.unwrapForShortestPath(s.getLatLng(LatLng::Wrapped));
        ScreenCoordinate point = s.latLngToScreenCoordinate(unwrappedLatLng);
        point.y = s.getSize().height - point.y;
        return point;
    }

    static Point<double> project(const LatLng& c, const TransformState& s) {
        LatLng unwrappedLatLng = c.wrapped();
        unwrappedLatLng.unwrapForShortestPath(s.getLatLng(LatLng::Wrapped));
        return Projection::project(unwrappedLatLng, s.getScale());
    }

    static Point<double> unproject(const LatLng& c, const TransformState& s) {
        LatLng unwrappedLatLng = c.wrapped();
        unwrappedLatLng.unwrapForShortestPath(s.getLatLng(LatLng::Wrapped));
        return Projection::project(unwrappedLatLng, s.getScale());
    }

    void updateRadius(const mbgl::LocationIndicatorRenderParameters& params) {
#ifdef MLN_DRAWABLE_LOCATION_INDICATOR
        auto& circle = circleDrawableInfo.geometry;
        circleDrawableInfo.dirty = true;
#endif

        const TransformState& s = *params.state;
        const auto numVtxCircumference = static_cast<unsigned long>(circle.size() - 1);
        const float bearingStep = 360.0f /
                                  static_cast<float>(numVtxCircumference - 1); // first and last points are the same
        const mapbox::cheap_ruler::point centerPoint(params.puckPosition.longitude(), params.puckPosition.latitude());
        Point<double> center = project(params.puckPosition, s);
        circle[0] = {0, 0};

        const auto mapBearing = static_cast<float>(util::wrap(util::rad2deg(params.bearing), 0.0, util::DEGREES_MAX));
        for (unsigned long i = 1; i <= numVtxCircumference; ++i) {
            const float bearing_ = static_cast<float>(i - 1) * bearingStep - mapBearing;
            Point<double> poc = ruler.destination(centerPoint, params.errorRadiusMeters, bearing_);
            circle[i] = vec2(project(LatLng(poc.y, poc.x), s) - center);
        }
        radiusChanged = false;
    }

    // Size in "map pixels" for a screen pixel
    static float pixelSizeToWorldSizeH(const LatLng& pos, const TransformState& s) {
        ScreenCoordinate posScreen = latLngToScreenCoordinate(pos, s);
        ScreenCoordinate posScreenLeftPx = posScreen;
        posScreenLeftPx.x -= 1;
        LatLng posLeftPx = screenCoordinateToLatLng(posScreenLeftPx, s);
        Point<double> posMerc = project(pos, s);
        Point<double> posLeftPxMerc = project(posLeftPx, s);
        return vec2(posMerc - posLeftPxMerc).length();
    }

    static vec2 verticalDirectionMercator(const LatLng& pos, const TransformState& s) {
        ScreenCoordinate posScreen = latLngToScreenCoordinate(pos, s);
        Point<double> posMerc = project(pos, s);
        return verticalDirectionMercator(posScreen, posMerc, s);
    }

    static vec2 verticalDirectionMercator(const ScreenCoordinate& pos, Point<double> posMerc, const TransformState& s) {
        ScreenCoordinate screenDy = pos;
        screenDy.y -= 1;
        LatLng posDy = screenCoordinateToLatLng(screenDy, s);
        Point<double> posMercDy = project(posDy, s);
        return verticalDirectionMercator(posMerc, posMercDy);
    }

    static vec2 verticalDirectionMercator(const Point<double>& posMerc, const Point<double>& posMercDy) {
        Point<double> verticalShiftMercator = posMercDy - posMerc;
        vec2 res(verticalShiftMercator);
        return res.normalized();
    }

    static Point<double> hatShadowShiftVector(const LatLng& position,
                                              const mbgl::LocationIndicatorRenderParameters& params) {
        const TransformState& s = *params.state;
        ScreenCoordinate posScreen = latLngToScreenCoordinate(position, s);
        posScreen.y = params.height - 1; // moving it to bottom
        Point<double> posMerc = project(screenCoordinateToLatLng(posScreen, s), s);
        vec2 verticalShiftAtPos = verticalDirectionMercator(posScreen, posMerc, s);
        return {verticalShiftAtPos.x, verticalShiftAtPos.y};
    }

    static vec2 directionAtPositionScreen(const LatLng& position, float bearing, const TransformState& s) {
        const double scale = s.getScale();
        const vec2 rot = vec2(0.0, -1.0).rotated(-bearing);
        Point<double> posMerc = project(position, s);
        Point<double> posMercDelta = posMerc + rot.toPoint();
        ScreenCoordinate posScreen = latLngToScreenCoordinate(position, s);
        ScreenCoordinate posScreenDelta = latLngToScreenCoordinate(Projection::unproject(posMercDelta, scale), s);
        return vec2(posScreenDelta - posScreen).normalized();
    }

    void updatePuck(const mbgl::LocationIndicatorRenderParameters& params) {
        updatePuckPerspective(params);
        bearingChanged = false;
    }

    void updatePuckPerspective(const mbgl::LocationIndicatorRenderParameters& params) {
        const TransformState& s = *params.state;
        projectionPuck = projectionCircle; // Duplicated as it might change, depending
                                           // on what puck style is chosen.
        const mapbox::cheap_ruler::point centerPoint(params.puckPosition.longitude(), params.puckPosition.latitude());
        static constexpr float bearings[]{
            225.0f, 315.0f, 45.0f, 135.0f}; // Quads will be drawn as triangle fans. so bl, tl, tr, br
#ifndef M_SQRT2
        static constexpr const float M_SQRT2 = std::sqrt(2.0f);
#endif
        // The puck has to stay square at all zoom levels. CheapRuler::destination
        // does not guarantee this at low zoom levels, so the extent has to be produced in mercator space
        const double tilt = s.getPitch();

        // Point<double> verticalShiftAtCenter { float(std::sin(util::DEG2RAD_D
        // * util::wrap<float>(-t.getBearing() * util::RAD2DEG, 0.0f, 360.0f)
        // )),
        //                                      -float(std::cos(util::DEG2RAD_D
        //                                      * util::wrap<float>(-t.getBearing() *
        //                                      util::RAD2DEG_D, 0.0f, 360.0f)))
        //                                      };
        // would be correct only in the vertical center of the map. As soon as
        // position goes away from that line, the shift direction is skewed by
        // the perspective projection. So the way to have a shift aligned to the
        // screen vertical axis is to find this direction in screen space, and
        // convert it back to map space. This would yield an always straight up
        // shift. However, going further (= the opposite direction of where the
        // lines are converging in the projection) might produce an even more
        // realistic effect. But in this case, it empirically seems that the
        // largest shift that look acceptable is what is obtained at the bottom
        // of the window, avoiding the wider converging lines that pass by the
        // edge of the screen going toward the top.

        Point<double> verticalShift = hatShadowShiftVector(params.puckPosition, params);
        const float horizontalScaleFactor =
            (1.0f - params.perspectiveCompensation) +
            util::clamp(pixelSizeToWorldSizeH(params.puckPosition, s), 0.8f, 100.1f) *
                params.perspectiveCompensation; // Compensation factor for the perspective deformation
        //     ^ clamping this to 0.8 to avoid growing the puck too much close to the camera.

#ifndef MLN_DRAWABLE_LOCATION_INDICATOR
        const double shadowRadius = ((texShadow) ? texShadow->width / texShadow->pixelRatio : 0.0) *
                                    params.puckShadowScale * M_SQRT2 * 0.5 *
                                    horizontalScaleFactor; // Technically it's not the radius, but
                                                           // the half diagonal of the quad.
        const double puckRadius = ((texPuck) ? texPuck->width / texPuck->pixelRatio : 0.0) * params.puckScale *
                                  M_SQRT2 * 0.5 * horizontalScaleFactor;
        const double hatRadius = ((texPuckHat) ? texPuckHat->width / texPuckHat->pixelRatio : 0.0) *
                                 params.puckHatScale * M_SQRT2 * 0.5 * horizontalScaleFactor;
#else
        const double shadowScale = shadowDrawableInfo.textureInfo.width / shadowDrawableInfo.textureInfo.pixelRatio;
        const double shadowRadius = shadowScale * params.puckShadowScale * M_SQRT2 * 0.5 * horizontalScaleFactor;

        const double puckScale = puckDrawableInfo.textureInfo.width / puckDrawableInfo.textureInfo.pixelRatio;
        const double puckRadius = puckScale * params.puckScale * M_SQRT2 * 0.5 * horizontalScaleFactor;

        const double hatScale = hatDrawableInfo.textureInfo.width / hatDrawableInfo.textureInfo.pixelRatio;
        const double hatRadius = hatScale * params.puckHatScale * M_SQRT2 * 0.5 * horizontalScaleFactor;

        auto& shadowGeometry = shadowDrawableInfo.geometry;
        auto& puckGeometry = puckDrawableInfo.geometry;
        auto& hatGeometry = hatDrawableInfo.geometry;

        shadowDrawableInfo.dirty = true;
        puckDrawableInfo.dirty = true;
        hatDrawableInfo.dirty = true;
#endif

        for (unsigned long i = 0; i < 4; ++i) {
            const auto b = util::wrap<float>(static_cast<float>(params.puckBearing) + bearings[i], 0.0f, 360.0f);

            const Point<double> cornerDirection{std::sin(util::deg2rad(b)), -std::cos(util::deg2rad(b))};

            Point<double> shadowOffset = cornerDirection * shadowRadius;
            Point<double> puckOffset = cornerDirection * puckRadius;
            Point<double> hatOffset = cornerDirection * hatRadius;

            shadowGeometry[i] = vec2(shadowOffset +
                                     (verticalShift * (tilt * -params.puckLayersDisplacement * horizontalScaleFactor)));
            puckGeometry[i] = vec2(puckOffset);
            hatGeometry[i] = vec2(hatOffset +
                                  (verticalShift * (tilt * params.puckLayersDisplacement * horizontalScaleFactor)));
        }
    }

#ifndef MLN_DRAWABLE_LOCATION_INDICATOR
    void drawRadius(const mbgl::LocationIndicatorRenderParameters& params) {
        if (!(params.errorRadiusMeters > 0.0) ||
            (params.errorRadiusColor.a == 0.0 && params.errorRadiusBorderColor.a == 0.0))
            return;

        simpleShader.bind();
        mbgl::gl::bindUniform(simpleShader.u_color, params.errorRadiusColor);
        mbgl::gl::bindUniform(simpleShader.u_matrix, projectionCircle);

        circleBuffer.upload(circle);
        MBGL_CHECK_ERROR(glEnableVertexAttribArray(simpleShader.a_pos));
        MBGL_CHECK_ERROR(glVertexAttribPointer(simpleShader.a_pos, 2, GL_FLOAT, GL_FALSE, 0, nullptr));

        MBGL_CHECK_ERROR(glDrawArrays(GL_TRIANGLE_FAN, 0, GLsizei(circle.size())));
        if (params.errorRadiusBorderColor.a > 0.0f) {
            mbgl::gl::bindUniform(simpleShader.u_color, params.errorRadiusBorderColor);
            MBGL_CHECK_ERROR(glLineWidth(1.0f));
            MBGL_CHECK_ERROR(glDrawArrays(GL_LINE_STRIP, 1, GLsizei(circle.size() - 1)));
        }
        MBGL_CHECK_ERROR(glDisableVertexAttribArray(simpleShader.a_pos));
        circleBuffer.detach();
        simpleShader.detach();
    }

    void drawQuad(Buffer& buf, std::array<vec2, 4>& data, std::shared_ptr<Texture>& texture) {
        if (!texture || !texture->isValid()) return;
        texturedShader.bind();
        texture->bind(0);
        glUniform1i(texturedShader.u_image, 0);
        mbgl::gl::bindUniform(texturedShader.u_matrix, projectionPuck);

        buf.bind();
        buf.upload(data);
        MBGL_CHECK_ERROR(glEnableVertexAttribArray(texturedShader.a_pos));
        MBGL_CHECK_ERROR(glVertexAttribPointer(texturedShader.a_pos, 2, GL_FLOAT, GL_FALSE, 0, nullptr));

        texCoordsBuffer.bind();
        MBGL_CHECK_ERROR(glEnableVertexAttribArray(texturedShader.a_texCoord));
        MBGL_CHECK_ERROR(glVertexAttribPointer(texturedShader.a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, nullptr));

        MBGL_CHECK_ERROR(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
        texture->detach();
        texCoordsBuffer.detach();
        texturedShader.detach();
    }

    void drawShadow() { drawQuad(shadowBuffer, shadowGeometry, texShadow); }

    void drawPuck() { drawQuad(puckBuffer, puckGeometry, texPuck); }

    void drawHat() { drawQuad(hatBuffer, hatGeometry, texPuckHat); }
#endif

    static LatLng screenCoordinateToLatLng(const ScreenCoordinate& p,
                                           const TransformState& s,
                                           LatLng::WrapMode wrapMode = LatLng::Wrapped) {
        ScreenCoordinate flippedPoint = p;
        flippedPoint.y = s.getSize().height - flippedPoint.y;
        return s.screenCoordinateToLatLng(flippedPoint, wrapMode);
    }

#ifndef MLN_DRAWABLE_LOCATION_INDICATOR
    bool setTextureFromImageID(const std::string& imagePath,
                               std::shared_ptr<Texture>& texture,
                               const mbgl::LocationIndicatorRenderParameters& params) {
        bool updated = false;
        if (textures.find(imagePath) == textures.end()) {
            std::shared_ptr<Texture> tx = std::make_shared<Texture>();
            if (!imagePath.empty() && params.imageManager) {
                tx->assign(params.imageManager->getSharedImage(imagePath));
                updated = true;
            } else {
                tx->assign(nullptr);
            }
            textures[imagePath] = tx;
            texture = tx;
        } else {
            const Immutable<style::Image::Impl>* sharedImage = params.imageManager->getSharedImage(imagePath);
            const mbgl::PremultipliedImage* img = (sharedImage) ? &sharedImage->get()->image : nullptr;
            std::shared_ptr<Texture>& tex = textures.at(imagePath);
            if (tex->image != img) { // image for the ID might have changed.
                tex->assign(sharedImage);
                updated = true;
            }
            texture = tex;
        }
        return updated;
    }

    std::map<std::string, std::shared_ptr<Texture>> textures;
    SimpleShader simpleShader;
    TexturedShader texturedShader;
    Buffer buffer;
    Buffer circleBuffer;
    Buffer shadowBuffer;
    Buffer puckBuffer;
    Buffer hatBuffer;
    Buffer texCoordsBuffer;
    std::shared_ptr<Texture> texShadow;
    std::shared_ptr<Texture> texPuck;
    std::shared_ptr<Texture> texPuckHat;

    std::array<vec2, 73> circle; // 72 points + position
    std::array<vec2, 4> shadowGeometry;
    std::array<vec2, 4> puckGeometry;
    std::array<vec2, 4> hatGeometry;
    std::array<vec2, 4> texCoords;
#endif

    mapbox::cheap_ruler::CheapRuler ruler;

    mbgl::mat4 translation;
    mbgl::mat4 projectionCircle;
    mbgl::mat4 projectionPuck;

    bool positionChanged = false;
    bool radiusChanged = false;
    bool bearingChanged = false;
    mbgl::LocationIndicatorRenderParameters oldParams;
    bool initialized = false;
    bool dirtyFeature = true;

#ifdef MLN_DRAWABLE_LOCATION_INDICATOR

public:
    struct QuadDrawableInfo {
        std::optional<std::reference_wrapper<gfx::Drawable>> drawable;
        std::array<vec2, 4> geometry;
        TextureInfo textureInfo;
        bool dirty{false};

        gfx::Drawable& getDrawable() { return drawable.value().get(); }
        void reset() { textureInfo.reset(); }
    };

    struct CircleDrawableInfo {
        std::optional<std::reference_wrapper<gfx::Drawable>> drawable;
        std::optional<std::reference_wrapper<gfx::Drawable>> outlineDrawable;
        std::array<vec2, 73> geometry;
        TextureInfo textureInfo;
        bool dirty{false};

        gfx::Drawable& getDrawable() { return drawable.value().get(); }
        void reset() { textureInfo.reset(); }
    };

    CircleDrawableInfo circleDrawableInfo;
    QuadDrawableInfo shadowDrawableInfo;
    QuadDrawableInfo puckDrawableInfo;
    QuadDrawableInfo hatDrawableInfo;

    const auto& getProjectionCircle() const { return projectionCircle; }
    const auto& getProjectionPuck() const { return projectionPuck; }

#endif

public:
    mbgl::LocationIndicatorRenderParameters parameters;
    std::shared_ptr<mbgl::Feature> feature;
    std::shared_ptr<mapbox::geometry::polygon<int64_t>> featureEnvelope;
    static bool anisotropicFilteringAvailable;
};

bool RenderLocationIndicatorImpl::anisotropicFilteringAvailable = false;

using namespace style;
namespace {

inline const LocationIndicatorLayer::Impl& impl(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == LocationIndicatorLayer::Impl::staticTypeInfo());
    return static_cast<const LocationIndicatorLayer::Impl&>(*impl);
}
} // namespace

RenderLocationIndicatorLayer::RenderLocationIndicatorLayer(Immutable<style::LocationIndicatorLayer::Impl> _impl)
    : RenderLayer(makeMutable<LocationIndicatorLayerProperties>(std::move(_impl))),
      renderImpl(std::make_unique<RenderLocationIndicatorImpl>(impl(baseImpl).id)),
      unevaluated(impl(baseImpl).paint.untransitioned()) {
    assert(gfx::BackendScope::exists());
}

RenderLocationIndicatorLayer::~RenderLocationIndicatorLayer() {
    if (!contextDestroyed) renderImpl->release();
}

void RenderLocationIndicatorLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
    styleDependencies = unevaluated.getDependencies();
}

void RenderLocationIndicatorLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    passes = RenderPass::Translucent;
    auto properties = makeMutable<LocationIndicatorLayerProperties>(
        staticImmutableCast<LocationIndicatorLayer::Impl>(baseImpl), unevaluated.evaluate(parameters));
    const auto& evaluated = properties->evaluated;
    auto& layout = impl(baseImpl).layout;

    properties->renderPasses = mbgl::underlying_type(passes);

    // paint
    renderImpl->parameters.errorRadiusColor = evaluated.get<style::AccuracyRadiusColor>();
    renderImpl->parameters.errorRadiusBorderColor = evaluated.get<style::AccuracyRadiusBorderColor>();
    renderImpl->parameters.errorRadiusMeters = evaluated.get<style::AccuracyRadius>();
    renderImpl->parameters.puckScale = evaluated.get<style::BearingImageSize>();
    renderImpl->parameters.puckHatScale = evaluated.get<style::TopImageSize>();
    renderImpl->parameters.puckShadowScale = evaluated.get<style::ShadowImageSize>();
    renderImpl->parameters.puckBearing = evaluated.get<style::Bearing>().getAngle();
    renderImpl->parameters.puckLayersDisplacement = evaluated.get<style::ImageTiltDisplacement>();
    renderImpl->parameters.perspectiveCompensation = evaluated.get<style::PerspectiveCompensation>();

    const std::array<double, 3> pos = evaluated.get<style::Location>();
    renderImpl->parameters.puckPosition = LatLng{pos[0], pos[1]};

    // layout
    if (!layout.get<style::BearingImage>().isUndefined())
        renderImpl->parameters.puckImagePath = layout.get<style::BearingImage>().asConstant().id();
    if (!layout.get<style::ShadowImage>().isUndefined())
        renderImpl->parameters.puckShadowImagePath = layout.get<style::ShadowImage>().asConstant().id();
    if (!layout.get<style::TopImage>().isUndefined())
        renderImpl->parameters.puckHatImagePath = layout.get<style::TopImage>().asConstant().id();

    evaluatedProperties = std::move(properties);
}

bool RenderLocationIndicatorLayer::hasTransition() const {
    return unevaluated.hasTransition();
}
bool RenderLocationIndicatorLayer::hasCrossfade() const {
    return false;
}

void RenderLocationIndicatorLayer::markContextDestroyed() {
    contextDestroyed = true;
}

void RenderLocationIndicatorLayer::prepare(const LayerPrepareParameters& p) {
    renderImpl->parameters.imageManager = &p.imageManager;
    const TransformState& state = p.state;
    renderImpl->parameters.state = &state;

    renderImpl->parameters.width = state.getSize().width;
    renderImpl->parameters.height = state.getSize().height;
    renderImpl->parameters.latitude = state.getLatLng().latitude();
    renderImpl->parameters.longitude = state.getLatLng().longitude();
    renderImpl->parameters.zoom = state.getZoom();
    renderImpl->parameters.bearing = util::rad2deg(-state.getBearing());
    renderImpl->parameters.pitch = state.getPitch();
    mat4 projMatrix;
    state.getProjMatrix(projMatrix);
    renderImpl->parameters.projectionMatrix = projMatrix;

    renderImpl->updatePuckGeometry(renderImpl->parameters);
}

void RenderLocationIndicatorLayer::populateDynamicRenderFeatureIndex(DynamicFeatureIndex& index) const {
    renderImpl->updateFeature();
    if (!renderImpl->featureEnvelope->empty()) index.insert(renderImpl->feature, renderImpl->featureEnvelope);
}

#ifndef MLN_DRAWABLE_LOCATION_INDICATOR
void RenderLocationIndicatorLayer::render(PaintParameters& paintParameters) {
    auto& glContext = static_cast<gl::Context&>(paintParameters.context);

    // Reset GL state to a known state so the CustomLayer always has a clean slate.
    glContext.bindVertexArray = 0;
    glContext.setDepthMode(paintParameters.depthModeForSublayer(0, gfx::DepthMaskType::ReadOnly));
    glContext.setStencilMode(gfx::StencilMode::disabled());
    glContext.setColorMode(paintParameters.colorModeForRenderPass()); // this is gfx::ColorMode::alphaBlended()
    glContext.setCullFaceMode(gfx::CullFaceMode::disabled());

    MBGL_CHECK_ERROR(renderImpl->render(renderImpl->parameters));

    // Reset the view back to our original one, just in case the CustomLayer
    // changed the viewport or Framebuffer.
    paintParameters.backend.getDefaultRenderable().getResource<gl::RenderableResource>().bind();
    glContext.setDirtyState();
}
#endif

#ifdef MLN_DRAWABLE_LOCATION_INDICATOR

void RenderLocationIndicatorLayer::update(gfx::ShaderRegistry& shaders,
                                          gfx::Context& context,
                                          const TransformState&,
                                          const std::shared_ptr<UpdateParameters>&,
                                          const RenderTree&,
                                          UniqueChangeRequestVec& changes) {
    const auto drawPasses = RenderPass::Translucent;

    // If the result is transparent or missing, just remove any existing drawables and stop
    if (drawPasses == RenderPass::None) {
        removeAllDrawables();
        return;
    }

    if (!quadShader) {
        quadShader = context.getGenericShader(shaders, "LocationIndicatorTexturedShader");
    }

    if (!quadShader) {
        removeAllDrawables();
        return;
    }

    if (!circleShader) {
        circleShader = context.getGenericShader(shaders, "LocationIndicatorShader");
    }

    if (!circleShader) {
        removeAllDrawables();
        return;
    }

    if (!layerGroup) {
        if (auto layerGroup_ = context.createLayerGroup(layerIndex, /*initialCapacity=*/4, getID())) {
            setLayerGroup(std::move(layerGroup_), changes);
        } else {
            return;
        }
    }

    if (!layerTweaker) {
        layerTweaker = std::make_shared<LocationIndicatorLayerTweaker>(
            getID(), evaluatedProperties, renderImpl->getProjectionCircle(), renderImpl->getProjectionPuck());
        layerGroup->addLayerTweaker(layerTweaker);
    }

    auto* localLayerGroup = static_cast<LayerGroup*>(layerGroup.get());

    if (localLayerGroup->getDrawableCount() == 0) {
        // create empty drawable using a builder
        const gfx::UniqueDrawableBuilder& builder = context.createDrawableBuilder(getID());

        const auto createQuadGeometry = [&](gfx::Drawable& drawable, const auto& geometry) {
            auto vertexAttrs = context.createVertexAttributeArray();

            drawable.setVertices({}, 4, gfx::AttributeDataType::Float2);
            vertexAttrs->set(shaders::idLocationIndicatorPosVertexAttribute, 0, gfx::AttributeDataType::Float2, 4);

            if (const auto& attr = vertexAttrs->set(
                    shaders::idLocationIndicatorTexVertexAttribute, 0, gfx::AttributeDataType::Float2, 4)) {
                const std::array<RenderLocationIndicatorImpl::vec2, 4> texCoords = {
                    {{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}}};

                auto geoDataPtr = reinterpret_cast<const uint8_t*>(texCoords.data());
                auto geoDataSize = texCoords.size() * sizeof(RenderLocationIndicatorImpl::vec2);

                std::vector<uint8_t> rawData;
                std::copy(geoDataPtr, geoDataPtr + geoDataSize, std::back_inserter(rawData));
                attr->setRawData(std::move(rawData));
            }

            drawable.setVertexAttributes(vertexAttrs);

            gfx::IndexVector<gfx::Triangles> indices;
            indices.emplace_back(0, 1, 2);
            indices.emplace_back(0, 2, 3);

            std::vector<gfx::Drawable::UniqueDrawSegment> drawSegments;
            drawSegments.emplace_back(
                builder->createSegment(gfx::Triangles(), SegmentBase(0, 0, geometry.size(), indices.elements())));
            drawable.setIndexData(indices.vector(), std::move(drawSegments));
        };

        const auto createQuadDrawable = [&](RenderLocationIndicatorImpl::QuadDrawableInfo& drawableInfo,
                                            std::string&& name,
                                            LocationIndicatorComponentType type) {
            auto& drawable = builder->getCurrentDrawable(true);
            drawable->setType(static_cast<uint8_t>(type));

            drawable->setName(name);
            drawable->setRenderPass(drawPasses);
            drawable->setDepthType(gfx::DepthMaskType::ReadOnly);
            drawable->setEnableDepth(false);
            drawable->setEnableStencil(false);
            drawable->setColorMode(drawPasses == RenderPass::Translucent ? gfx::ColorMode::alphaBlended()
                                                                         : gfx::ColorMode::unblended());

            drawable->setShader(quadShader);

            createQuadGeometry(*drawable, drawableInfo.geometry);

            drawableInfo.drawable.emplace(*drawable);

            // add drawable to layer group
            localLayerGroup->addDrawable(std::move(drawable));
            ++stats.drawablesAdded;
        };

        // create circle drawable
        const auto getCircleDrawable = [&](const auto& name, auto& vertexAttrib) -> gfx::UniqueDrawable& {
            auto& drawable = builder->getCurrentDrawable(true);

            drawable->setName(name);
            drawable->setRenderPass(drawPasses);
            drawable->setDepthType(gfx::DepthMaskType::ReadOnly);
            drawable->setEnableDepth(false);
            drawable->setEnableStencil(false);
            drawable->setColorMode(drawPasses == RenderPass::Translucent ? gfx::ColorMode::alphaBlended()
                                                                         : gfx::ColorMode::unblended());

            drawable->setShader(circleShader);
            drawable->setVertexAttributes(vertexAttrib);

            return drawable;
        };

        const auto createCircleDrawable = [&]() {
            // circle[0] is the center
            const auto& geometry = renderImpl->circleDrawableInfo.geometry;
            const uint16_t vertexCount = static_cast<uint16_t>(geometry.size());
            auto vertexAttrib = context.createVertexAttributeArray();

            {
                auto& drawable = getCircleDrawable("locationAccuracyCircle", vertexAttrib);
                drawable->setType(static_cast<uint8_t>(LocationIndicatorComponentType::Circle));

                std::vector<gfx::Drawable::UniqueDrawSegment> drawSegments;
                std::vector<uint16_t> indices;

                for (uint16_t i = 1; i < vertexCount - 1; ++i) {
                    indices.insert(indices.end(), {0, i, static_cast<uint16_t>(i + 1)});
                }
                indices.insert(indices.end(), {0, static_cast<uint16_t>(vertexCount - 1), 1});

                SegmentBase segment(0, 0, vertexCount, indices.size());
                drawSegments.emplace_back(builder->createSegment(gfx::Triangles(), std::move(segment)));
                drawable->setIndexData(indices, std::move(drawSegments));

                renderImpl->circleDrawableInfo.drawable.emplace(*drawable);

                localLayerGroup->addDrawable(std::move(drawable));
                ++stats.drawablesAdded;
            }

            {
                auto& drawable = getCircleDrawable("locationAccuracyCircleOutline", vertexAttrib);
                drawable->setType(static_cast<uint8_t>(LocationIndicatorComponentType::CircleOutline));

                std::vector<gfx::Drawable::UniqueDrawSegment> drawSegments;
                std::vector<uint16_t> indices;

                for (uint16_t i = 1; i < vertexCount; ++i) {
                    indices.push_back(i);
                }

                SegmentBase segment(0, 0, vertexCount, indices.size());
                drawSegments.emplace_back(builder->createSegment(gfx::LineStrip(1.0f), std::move(segment)));
                drawable->setIndexData(indices, std::move(drawSegments));

                renderImpl->circleDrawableInfo.outlineDrawable.emplace(*drawable);

                localLayerGroup->addDrawable(std::move(drawable));
                ++stats.drawablesAdded;
            }
        };

        createCircleDrawable();
        createQuadDrawable(
            renderImpl->shadowDrawableInfo, "locationShadow", LocationIndicatorComponentType::PuckShadow);
        createQuadDrawable(renderImpl->puckDrawableInfo, "locationPuck", LocationIndicatorComponentType::Puck);
        createQuadDrawable(renderImpl->hatDrawableInfo, "locationPuckHat", LocationIndicatorComponentType::PuckHat);
    };

    const auto updateCircleDrawable = [&]() {
        auto& circleDrawable = renderImpl->circleDrawableInfo.drawable.value().get();
        auto& circleOutlineDrawable = renderImpl->circleDrawableInfo.outlineDrawable.value().get();

        if (!(renderImpl->parameters.errorRadiusMeters > 0.0) ||
            (renderImpl->parameters.errorRadiusColor.a == 0.0 &&
             renderImpl->parameters.errorRadiusBorderColor.a == 0.0)) {
            circleDrawable.setEnabled(false);
            circleOutlineDrawable.setEnabled(false);
            return;
        } else {
            circleDrawable.setEnabled(true);
            circleOutlineDrawable.setEnabled(true);
        }

        // update attributes
        if (renderImpl->circleDrawableInfo.dirty) {
            // vertex attribute data is shared between the 2 circle drawables
            const auto& geometry = renderImpl->circleDrawableInfo.geometry;

            using VertexVector = gfx::VertexVector<RenderLocationIndicatorImpl::vec2>;
            std::shared_ptr<VertexVector> verts = std::make_shared<VertexVector>();
            for (const auto& elem : geometry) {
                verts->emplace_back(elem);
            }

            auto& circleVertexAttrs = circleDrawable.getVertexAttributes();
            if (const auto& attr = circleVertexAttrs->set(shaders::idLocationIndicatorPosVertexAttribute)) {
                attr->setSharedRawData(
                    verts, 0, 0, sizeof(RenderLocationIndicatorImpl::vec2), gfx::AttributeDataType::Float2);
            }
        }
    };

    const auto updateQuadDrawable = [&](RenderLocationIndicatorImpl::QuadDrawableInfo& info) {
        auto& drawable = info.getDrawable();

        if (info.dirty) {
            auto vertexAttrs = drawable.getVertexAttributes();

            if (const auto& attr = vertexAttrs->set(
                    shaders::idLocationIndicatorPosVertexAttribute, 0, gfx::AttributeDataType::Float2)) {
                auto geoDataPtr = reinterpret_cast<const uint8_t*>(info.geometry.data());
                auto geoDataSize = info.geometry.size() * sizeof(RenderLocationIndicatorImpl::vec2);

                std::vector<uint8_t> rawData;
                std::copy(geoDataPtr, geoDataPtr + geoDataSize, std::back_inserter(rawData));
                attr->setRawData(std::move(rawData));
            }

            info.dirty = false;
        }

        if (info.textureInfo.dirty) {
            if (info.textureInfo.image) {
                if (!info.textureInfo.texture) {
                    info.textureInfo.texture = context.createTexture2D();
                    info.textureInfo.texture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Linear,
                                                                       .wrapU = gfx::TextureWrapType::Clamp,
                                                                       .wrapV = gfx::TextureWrapType::Clamp,
                                                                       .maxAnisotropy = 16,
                                                                       .mipmapped = true});
                }

                info.textureInfo.texture->upload(info.textureInfo.image->get()->image);
                info.textureInfo.image.reset();
            }

            drawable.setTexture(info.textureInfo.texture, shaders::idLocationIndicatorTexture);
            info.textureInfo.dirty = false;
        }
    };

    updateCircleDrawable();
    updateQuadDrawable(renderImpl->shadowDrawableInfo);
    updateQuadDrawable(renderImpl->puckDrawableInfo);
    updateQuadDrawable(renderImpl->hatDrawableInfo);
}

#endif

} // namespace mbgl
