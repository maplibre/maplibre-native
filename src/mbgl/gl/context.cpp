#include <mbgl/gl/context.hpp>

#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/gl/command_encoder.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/draw_scope_resource.hpp>
#include <mbgl/gl/enum.hpp>
#include <mbgl/gl/renderer_backend.hpp>
#include <mbgl/gl/renderbuffer_resource.hpp>
#include <mbgl/gl/offscreen_texture.hpp>
#include <mbgl/gl/debugging_extension.hpp>
#include <mbgl/gl/timestamp_query_extension.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/traits.hpp>
#include <mbgl/util/std.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/thread_pool.hpp>

#include <mbgl/gl/drawable_gl.hpp>
#include <mbgl/gl/drawable_gl_builder.hpp>
#include <mbgl/gl/layer_group_gl.hpp>
#include <mbgl/gl/uniform_buffer_gl.hpp>
#include <mbgl/gl/texture2d.hpp>
#include <mbgl/renderer/render_target.hpp>
#include <mbgl/shaders/gl/shader_program_gl.hpp>

#include <cstring>
#include <iterator>

namespace mbgl {
namespace gl {

using namespace platform;

static_assert(underlying_type(ShaderType::Vertex) == GL_VERTEX_SHADER, "OpenGL type mismatch");
static_assert(underlying_type(ShaderType::Fragment) == GL_FRAGMENT_SHADER, "OpenGL type mismatch");

static_assert(std::is_same_v<ProgramID, GLuint>, "OpenGL type mismatch");
static_assert(std::is_same_v<ShaderID, GLuint>, "OpenGL type mismatch");
static_assert(std::is_same_v<BufferID, GLuint>, "OpenGL type mismatch");
static_assert(std::is_same_v<TextureID, GLuint>, "OpenGL type mismatch");
static_assert(std::is_same_v<VertexArrayID, GLuint>, "OpenGL type mismatch");
static_assert(std::is_same_v<FramebufferID, GLuint>, "OpenGL type mismatch");
static_assert(std::is_same_v<RenderbufferID, GLuint>, "OpenGL type mismatch");

static_assert(underlying_type(UniformDataType::Float) == GL_FLOAT, "OpenGL type mismatch");
static_assert(underlying_type(UniformDataType::FloatVec2) == GL_FLOAT_VEC2, "OpenGL type mismatch");
static_assert(underlying_type(UniformDataType::FloatVec3) == GL_FLOAT_VEC3, "OpenGL type mismatch");
static_assert(underlying_type(UniformDataType::FloatVec4) == GL_FLOAT_VEC4, "OpenGL type mismatch");
static_assert(underlying_type(UniformDataType::Int) == GL_INT, "OpenGL type mismatch");
static_assert(underlying_type(UniformDataType::IntVec2) == GL_INT_VEC2, "OpenGL type mismatch");
static_assert(underlying_type(UniformDataType::IntVec3) == GL_INT_VEC3, "OpenGL type mismatch");
static_assert(underlying_type(UniformDataType::IntVec4) == GL_INT_VEC4, "OpenGL type mismatch");
static_assert(underlying_type(UniformDataType::Bool) == GL_BOOL, "OpenGL type mismatch");
static_assert(underlying_type(UniformDataType::BoolVec2) == GL_BOOL_VEC2, "OpenGL type mismatch");
static_assert(underlying_type(UniformDataType::BoolVec3) == GL_BOOL_VEC3, "OpenGL type mismatch");
static_assert(underlying_type(UniformDataType::BoolVec4) == GL_BOOL_VEC4, "OpenGL type mismatch");
static_assert(underlying_type(UniformDataType::FloatMat2) == GL_FLOAT_MAT2, "OpenGL type mismatch");
static_assert(underlying_type(UniformDataType::FloatMat3) == GL_FLOAT_MAT3, "OpenGL type mismatch");
static_assert(underlying_type(UniformDataType::FloatMat4) == GL_FLOAT_MAT4, "OpenGL type mismatch");
static_assert(underlying_type(UniformDataType::Sampler2D) == GL_SAMPLER_2D, "OpenGL type mismatch");
static_assert(underlying_type(UniformDataType::SamplerCube) == GL_SAMPLER_CUBE, "OpenGL type mismatch");

namespace {
GLint getMaxVertexAttribs() {
    GLint value = 0;
    MBGL_CHECK_ERROR(glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &value));
    return value;
}

// Currently renderBufferByteSize is only used when Tracy profiling is enabled
#ifdef MLN_TRACY_ENABLE
constexpr size_t renderBufferByteSize(const gfx::RenderbufferPixelType type, const Size size) noexcept {
    size_t sz = size.width * size.height;
    switch (type) {
        case gfx::RenderbufferPixelType::RGBA:
            [[fallthrough]];
        case gfx::RenderbufferPixelType::DepthStencil:
            sz *= 4;
            break;
        case gfx::RenderbufferPixelType::Depth:
            sz *= 2;
            break;
    }
    return sz;
}
#endif
} // namespace

Context::Context(RendererBackend& backend_)
    : gfx::Context(/*maximumVertexBindingCount=*/getMaxVertexAttribs()),
      backend(backend_) {
    uboAllocator = std::make_unique<gl::UniformBufferAllocator>();

    texturePool = std::make_unique<Texture2DPool>(this);
}

Context::~Context() noexcept {
    if (cleanupOnDestruction) {
        backend.getThreadPool().runRenderJobs(true /* closeQueue */);

        glUseProgram(0);

        for (size_t i = 0; i < globalUniformBuffers.allocatedSize(); i++) {
            globalUniformBuffers.set(i, nullptr);
        }

        reset();

        // Delete all pooled resources while the context is still valid
        texturePool.reset();
        uboAllocator.reset();

#if !defined(NDEBUG)
        Log::Debug(Event::General, "Rendering Stats:\n" + stats.toString("\n"));
#endif
        assert(stats.isZero());
    }
}

void Context::beginFrame() {
    MLN_TRACE_FUNC();

    backend.getThreadPool().runRenderJobs();

    frameInFlightFence = std::make_shared<gl::Fence>();

    // Run allocator defragmentation on this frame interval.
    constexpr auto defragFreq = 4;

    if (frameNum == defragFreq) {
        uboAllocator->defragment(frameInFlightFence);
        frameNum = 0;
    } else {
        frameNum++;
    }
}

void Context::endFrame() {
    MLN_TRACE_FUNC();

    if (!frameInFlightFence) {
        return;
    }

    frameInFlightFence->insert();
}

void Context::initializeExtensions(const std::function<gl::ProcAddress(const char*)>& getProcAddress) {
    MLN_TRACE_FUNC();

    if (const auto* extensions = reinterpret_cast<const char*>(MBGL_CHECK_ERROR(glGetString(GL_EXTENSIONS)))) {
        auto fn = [&](std::initializer_list<std::pair<const char*, const char*>> probes) -> ProcAddress {
            for (auto probe : probes) {
                if (strstr(extensions, probe.first) != nullptr) {
                    if (ProcAddress ptr = getProcAddress(probe.second)) {
                        return ptr;
                    }
                }
            }
            return nullptr;
        };

        static const std::string renderer = []() {
            std::string r = reinterpret_cast<const char*>(MBGL_CHECK_ERROR(glGetString(GL_RENDERER)));
            Log::Info(Event::General, "GPU Identifier: " + r);
            return r;
        }();

        // Block ANGLE on Direct3D since the debugging extension is causing crashes
        if (!(renderer.find("ANGLE") != std::string::npos && renderer.find("Direct3D") != std::string::npos)) {
            debugging = std::make_unique<extension::Debugging>(fn);
        }

// Currently GL timestamp queries are only used when Tracy profiling is enabled
#ifdef MLN_TRACY_ENABLE
        extension::loadTimeStampQueryExtension(fn);
#endif
    }
    MLN_TRACE_GL_CONTEXT();
}

void Context::enableDebugging() {
    if (!debugging || !debugging->debugMessageControl || !debugging->debugMessageCallback) {
        return;
    }

    // This will enable all messages including performance hints
    // MBGL_CHECK_ERROR(debugging->debugMessageControl(GL_DONT_CARE,
    // GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE));

    // This will only enable high and medium severity messages
    MBGL_CHECK_ERROR(
        debugging->debugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE));
    MBGL_CHECK_ERROR(
        debugging->debugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE));
    MBGL_CHECK_ERROR(debugging->debugMessageControl(
        GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE));

    MBGL_CHECK_ERROR(debugging->debugMessageCallback(extension::Debugging::DebugCallback, nullptr));
}

UniqueShader Context::createShader(ShaderType type, const std::initializer_list<const char*>& sources) {
    UniqueShader result{MBGL_CHECK_ERROR(glCreateShader(static_cast<GLenum>(type))), {this}};

    MBGL_CHECK_ERROR(glShaderSource(result, static_cast<GLsizei>(sources.size()), sources.begin(), nullptr));
    MBGL_CHECK_ERROR(glCompileShader(result));

    GLint status = 0;
    MBGL_CHECK_ERROR(glGetShaderiv(result, GL_COMPILE_STATUS, &status));
    if (status != 0) {
        return result;
    }

    GLint logLength;
    MBGL_CHECK_ERROR(glGetShaderiv(result, GL_INFO_LOG_LENGTH, &logLength));
    if (logLength > 0) {
        const auto log = std::make_unique<GLchar[]>(logLength);
        MBGL_CHECK_ERROR(glGetShaderInfoLog(result, logLength, &logLength, log.get()));
        Log::Error(Event::Shader, std::string("Shader failed to compile: ") + log.get());
    }

    throw std::runtime_error("shader failed to compile");
}

UniqueProgram Context::createProgram(ShaderID vertexShader, ShaderID fragmentShader, const char* location0AttribName) {
    UniqueProgram result{MBGL_CHECK_ERROR(glCreateProgram()), {this}};

    MBGL_CHECK_ERROR(glAttachShader(result, vertexShader));
    MBGL_CHECK_ERROR(glAttachShader(result, fragmentShader));

    // It is important to have attribute at position 0 enabled: conveniently,
    // position attribute is always first and always enabled. The integrity of
    // this assumption is verified in AttributeLocations::queryLocations and
    // AttributeLocations::getFirstAttribName.
    MBGL_CHECK_ERROR(glBindAttribLocation(result, 0, location0AttribName));

    linkProgram(result);

    return result;
}

void Context::linkProgram(ProgramID program_) {
    MLN_TRACE_FUNC();

    MBGL_CHECK_ERROR(glLinkProgram(program_));
    verifyProgramLinkage(program_);
}

void Context::verifyProgramLinkage(ProgramID program_) {
    MLN_TRACE_FUNC();

    GLint status;
    MBGL_CHECK_ERROR(glGetProgramiv(program_, GL_LINK_STATUS, &status));
    if (status == GL_TRUE) {
        return;
    }

    GLint logLength;
    MBGL_CHECK_ERROR(glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &logLength));
    const auto log = std::make_unique<GLchar[]>(logLength);
    if (logLength > 0) {
        MBGL_CHECK_ERROR(glGetProgramInfoLog(program_, logLength, &logLength, log.get()));
        Log::Error(Event::Shader, std::string("Program failed to link: ") + log.get());
    }

    throw std::runtime_error("program failed to link");
}

UniqueTexture Context::createUniqueTexture(const Size& size,
                                           gfx::TexturePixelType format,
                                           gfx::TextureChannelDataType type) {
    MLN_TRACE_FUNC();

    assert(texturePool);
    TextureID id = texturePool->alloc(size, format, type);

    // NOLINTNEXTLINE(performance-move-const-arg)
    return UniqueTexture{std::move(id), {this}};
}

VertexArray Context::createVertexArray() {
    MLN_TRACE_FUNC();

    VertexArrayID id = 0;
    MBGL_CHECK_ERROR(glGenVertexArrays(1, &id));
    // NOLINTNEXTLINE(performance-move-const-arg)
    UniqueVertexArray vao(std::move(id), {this});
    return {UniqueVertexArrayState(new VertexArrayState(std::move(vao)), VertexArrayStateDeleter{true})};
}

UniqueFramebuffer Context::createFramebuffer() {
    MLN_TRACE_FUNC();

    FramebufferID id = 0;
    MBGL_CHECK_ERROR(glGenFramebuffers(1, &id));
    stats.numFrameBuffers++;
    // NOLINTNEXTLINE(performance-move-const-arg)
    return UniqueFramebuffer{std::move(id), {this}};
}

std::unique_ptr<gfx::RenderbufferResource> Context::createRenderbufferResource(const gfx::RenderbufferPixelType type,
                                                                               const Size size) {
    MLN_TRACE_FUNC();

    RenderbufferID id = 0;
    MBGL_CHECK_ERROR(glGenRenderbuffers(1, &id));
    MLN_TRACE_ALLOC_RT(id, renderBufferByteSize(type, size));
    // NOLINTNEXTLINE(performance-move-const-arg)
    UniqueRenderbuffer renderbuffer{std::move(id), {this}};

    bindRenderbuffer = renderbuffer;
    MBGL_CHECK_ERROR(
        glRenderbufferStorage(GL_RENDERBUFFER, Enum<gfx::RenderbufferPixelType>::to(type), size.width, size.height));
    bindRenderbuffer = 0;

    return std::make_unique<gl::RenderbufferResource>(std::move(renderbuffer));
}

std::unique_ptr<uint8_t[]> Context::readFramebuffer(const Size size,
                                                    const gfx::TexturePixelType format,
                                                    const bool flip) {
    MLN_TRACE_FUNC();
    MLN_TRACE_FUNC_GL();

    const size_t stride = size.width * (format == gfx::TexturePixelType::RGBA ? 4 : 1);
    auto data = std::make_unique<uint8_t[]>(stride * size.height);

    // When reading data from the framebuffer, make sure that we are storing the
    // values tightly packed into the buffer to avoid buffer overruns.
    pixelStorePack = {1};

    MBGL_CHECK_ERROR(glReadPixels(
        0, 0, size.width, size.height, Enum<gfx::TexturePixelType>::to(format), GL_UNSIGNED_BYTE, data.get()));

    if (flip) {
        auto tmp = std::make_unique<uint8_t[]>(stride);
        uint8_t* rgba = data.get();
        for (int i = 0, j = size.height - 1; i < j; i++, j--) {
            std::memcpy(tmp.get(), rgba + i * stride, stride);
            std::memcpy(rgba + i * stride, rgba + j * stride, stride);
            std::memcpy(rgba + j * stride, tmp.get(), stride);
        }
    }

    return data;
}

namespace {

void checkFramebuffer() {
    MLN_TRACE_FUNC();

    GLenum status = MBGL_CHECK_ERROR(glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        switch (status) {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                throw std::runtime_error("Couldn't create framebuffer: incomplete attachment");
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                throw std::runtime_error(
                    "Couldn't create framebuffer: incomplete missing "
                    "attachment");
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                throw std::runtime_error("Couldn't create framebuffer: incomplete draw buffer");
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                throw std::runtime_error("Couldn't create framebuffer: incomplete read buffer");
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
            case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
                throw std::runtime_error("Couldn't create framebuffer: incomplete dimensions");
#endif

            case GL_FRAMEBUFFER_UNSUPPORTED:
                throw std::runtime_error("Couldn't create framebuffer: unsupported");
            default:
                throw std::runtime_error("Couldn't create framebuffer: other");
        }
    }
}

void bindDepthStencilRenderbuffer(const gfx::Renderbuffer<gfx::RenderbufferPixelType::DepthStencil>& depthStencil) {
    MLN_TRACE_FUNC();
    MLN_TRACE_FUNC_GL();

    auto& depthStencilResource = depthStencil.getResource<gl::RenderbufferResource>();
#ifdef GL_DEPTH_STENCIL_ATTACHMENT
    MBGL_CHECK_ERROR(glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilResource.renderbuffer));
#else
    MBGL_CHECK_ERROR(glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthStencilResource.renderbuffer));
    MBGL_CHECK_ERROR(glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilResource.renderbuffer));
#endif
}

} // namespace

Framebuffer Context::createFramebuffer(
    const gfx::Renderbuffer<gfx::RenderbufferPixelType::RGBA>& color,
    const gfx::Renderbuffer<gfx::RenderbufferPixelType::DepthStencil>& depthStencil) {
    MLN_TRACE_FUNC();

    if (color.getSize() != depthStencil.getSize()) {
        throw std::runtime_error("Renderbuffer size mismatch");
    }
    auto fbo = createFramebuffer();
    bindFramebuffer = fbo;

    auto& colorResource = color.getResource<gl::RenderbufferResource>();
    MBGL_CHECK_ERROR(
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorResource.renderbuffer));
    bindDepthStencilRenderbuffer(depthStencil);
    checkFramebuffer();
    return {color.getSize(), std::move(fbo)};
}

Framebuffer Context::createFramebuffer(const gfx::Renderbuffer<gfx::RenderbufferPixelType::RGBA>& color) {
    MLN_TRACE_FUNC();

    auto fbo = createFramebuffer();
    bindFramebuffer = fbo;
    auto& colorResource = color.getResource<gl::RenderbufferResource>();
    MBGL_CHECK_ERROR(
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorResource.renderbuffer));
    checkFramebuffer();
    return {color.getSize(), std::move(fbo)};
}

std::unique_ptr<gfx::OffscreenTexture> Context::createOffscreenTexture(const Size size,
                                                                       const gfx::TextureChannelDataType type) {
    MLN_TRACE_FUNC();

    return std::make_unique<gl::OffscreenTexture>(*this, size, type);
}

std::unique_ptr<gfx::DrawScopeResource> Context::createDrawScopeResource() {
    MLN_TRACE_FUNC();

    return std::make_unique<gl::DrawScopeResource>(createVertexArray());
}

void Context::reset() {
    MLN_TRACE_FUNC();

    performCleanup();

    // Delete all unused pooled resources
    texturePool->shrink();
}

void Context::resetState(gfx::DepthMode depthMode, gfx::ColorMode colorMode) {
    MLN_TRACE_FUNC();
    MLN_TRACE_FUNC_GL();

    // Reset GL state to a known state so the CustomLayer always has a clean slate.
    bindVertexArray = value::BindVertexArray::Default;
    setDepthMode(depthMode);
    setStencilMode(gfx::StencilMode::disabled());
    setColorMode(colorMode);
    setCullFaceMode(gfx::CullFaceMode::disabled());
    setScissorTest({0, 0, 0, 0});
}

bool Context::emplaceOrUpdateUniformBuffer(gfx::UniformBufferPtr& buffer,
                                           const void* data,
                                           std::size_t size,
                                           bool persistent) {
    MLN_TRACE_FUNC();

    if (buffer) {
        buffer->update(data, size);
        return false;
    } else {
        buffer = createUniformBuffer(data, size, persistent);
        return true;
    }
}

void Context::bindGlobalUniformBuffers(gfx::RenderPass&) const noexcept {
    MLN_TRACE_FUNC();

    globalUniformBuffers.bind();
}

void Context::unbindGlobalUniformBuffers(gfx::RenderPass&) const noexcept {
    MLN_TRACE_FUNC();

    globalUniformBuffers.unbind();
}

void Context::setDirtyState() {
    MLN_TRACE_FUNC();

    // Note: does not set viewport/bindFramebuffer to dirty
    // since they are handled separately in the view object.
    stencilFunc.setDirty();
    stencilMask.setDirty();
    stencilTest.setDirty();
    stencilOp.setDirty();
#if MLN_RENDER_BACKEND_OPENGL
    depthRange.setDirty();
#endif
    depthMask.setDirty();
    depthTest.setDirty();
    depthFunc.setDirty();
    blend.setDirty();
    blendEquation.setDirty();
    blendFunc.setDirty();
    blendColor.setDirty();
    colorMask.setDirty();
    clearDepth.setDirty();
    clearColor.setDirty();
    clearStencil.setDirty();
    cullFace.setDirty();
    cullFaceSide.setDirty();
    cullFaceWinding.setDirty();
    scissorTest.setDirty();
    program.setDirty();
    lineWidth.setDirty();
    activeTextureUnit.setDirty();
    pixelStorePack.setDirty();
    pixelStoreUnpack.setDirty();
    for (auto& tex : texture) {
        tex.setDirty();
    }
    vertexBuffer.setDirty();
    bindVertexArray.setDirty();
    globalVertexArrayState.setDirty();
}

gfx::UniqueDrawableBuilder Context::createDrawableBuilder(std::string name) {
    MLN_TRACE_FUNC();

    return std::make_unique<gl::DrawableGLBuilder>(std::move(name));
}

gfx::UniformBufferPtr Context::createUniformBuffer(const void* data,
                                                   std::size_t size,
                                                   bool /*persistent*/,
                                                   bool /*ssbo*/) {
    MLN_TRACE_FUNC();

    return std::make_shared<gl::UniformBufferGL>(*this, data, size, *uboAllocator);
}

gfx::UniqueUniformBufferArray Context::createLayerUniformBufferArray() {
    return std::make_unique<UniformBufferArrayGL>();
}

gfx::ShaderProgramBasePtr Context::getGenericShader(gfx::ShaderRegistry& shaders, const std::string& name) {
    MLN_TRACE_FUNC();

    auto shaderGroup = shaders.getShaderGroup(name);
    if (!shaderGroup) {
        return nullptr;
    }
    return std::static_pointer_cast<gfx::ShaderProgramBase>(shaderGroup->getOrCreateShader(*this, {}));
}

TileLayerGroupPtr Context::createTileLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) {
    MLN_TRACE_FUNC();

    return std::make_shared<TileLayerGroupGL>(layerIndex, initialCapacity, std::move(name));
}

LayerGroupPtr Context::createLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) {
    MLN_TRACE_FUNC();

    return std::make_shared<LayerGroupGL>(layerIndex, initialCapacity, std::move(name));
}

gfx::Texture2DPtr Context::createTexture2D() {
    MLN_TRACE_FUNC();

    return std::make_shared<gl::Texture2D>(*this);
}

RenderTargetPtr Context::createRenderTarget(const Size size, const gfx::TextureChannelDataType type) {
    MLN_TRACE_FUNC();

    return std::make_shared<RenderTarget>(*this, size, type);
}

Framebuffer Context::createFramebuffer(const gfx::Texture2D& color) {
    MLN_TRACE_FUNC();

    auto fbo = createFramebuffer();
    bindFramebuffer = fbo;
    MBGL_CHECK_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER,
                                            GL_COLOR_ATTACHMENT0,
                                            GL_TEXTURE_2D,
                                            static_cast<const gl::Texture2D&>(color).getTextureID(),
                                            0));
    checkFramebuffer();
    return {color.getSize(), std::move(fbo)};
}

gfx::VertexAttributeArrayPtr Context::createVertexAttributeArray() const {
    return std::make_shared<VertexAttributeArrayGL>();
}

void Context::clear(std::optional<mbgl::Color> color, std::optional<float> depth, std::optional<int32_t> stencil) {
    MLN_TRACE_FUNC();
    MLN_TRACE_FUNC_GL();

    GLbitfield mask = 0;

    if (color) {
        mask |= GL_COLOR_BUFFER_BIT;
        clearColor = *color;
        colorMask = value::ColorMask::Default;
    }

    if (depth) {
        mask |= GL_DEPTH_BUFFER_BIT;
        clearDepth = *depth;
        depthMask = value::DepthMask::Default;
    }

    if (stencil) {
        mask |= GL_STENCIL_BUFFER_BIT;
        clearStencil = *stencil;
        stencilMask = value::StencilMask::Default;
    }

    MBGL_CHECK_ERROR(glClear(mask));

    stats.numDrawCalls = 0;
    stats.numFrames++;
}

void Context::setCullFaceMode(const gfx::CullFaceMode& mode) {
    cullFace = mode.enabled;

    // These shouldn't need to be updated when face culling is disabled, but we
    // might end up having the same isssues with Adreno 2xx GPUs as noted in
    // Context::setDepthMode.
    cullFaceSide = mode.side;
    cullFaceWinding = mode.winding;
}

void Context::setScissorTest(const gfx::ScissorRect& rect) {
    scissorTest = rect;
}

void Context::setDepthMode(const gfx::DepthMode& depth) {
    if (depth.func == gfx::DepthFunctionType::Always && depth.mask != gfx::DepthMaskType::ReadWrite) {
        depthTest = false;

        // Workaround for rendering errors on Adreno 2xx GPUs. Depth-related
        // state should not matter when the depth test is disabled, but on these
        // GPUs it apparently does.
        // https://github.com/mapbox/mapbox-gl-native/issues/9164
        depthFunc = depth.func;
        depthMask = depth.mask;
#if MLN_RENDER_BACKEND_OPENGL
        depthRange = depth.range;
#endif
    } else {
        depthTest = true;
        depthFunc = depth.func;
        depthMask = depth.mask;
#if MLN_RENDER_BACKEND_OPENGL
        depthRange = depth.range;
#endif
    }
}

void Context::setStencilMode(const gfx::StencilMode& stencil) {
    if (stencil.test.is<gfx::StencilMode::Always>() && !stencil.mask) {
        stencilTest = false;
    } else {
        stencilTest = true;
        stencilMask = stencil.mask;
        stencilOp = {stencil.fail, stencil.depthFail, stencil.pass};
        apply_visitor([&](const auto& test) { stencilFunc = {test.func, stencil.ref, test.mask}; }, stencil.test);
    }
}

void Context::setColorMode(const gfx::ColorMode& color) {
    if (color.blendFunction.is<gfx::ColorMode::Replace>()) {
        blend = false;
    } else {
        blend = true;
        blendColor = color.blendColor;
        apply_visitor(
            [&](const auto& blendFunction) {
                blendEquation = gfx::ColorBlendEquationType(blendFunction.equation);
                blendFunc = {blendFunction.srcFactor, blendFunction.dstFactor};
            },
            color.blendFunction);
    }

    colorMask = color.mask;
}

std::unique_ptr<gfx::CommandEncoder> Context::createCommandEncoder() {
    MLN_TRACE_FUNC();

    backend.updateAssumedState();
    if (backend.contextIsShared()) {
        setDirtyState();
    }
    return std::make_unique<gl::CommandEncoder>(*this);
}

void Context::finish() {
    MLN_TRACE_FUNC();
    MLN_TRACE_FUNC_GL();

    MBGL_CHECK_ERROR(glFinish());
}

std::shared_ptr<gl::Fence> Context::getCurrentFrameFence() const {
    return frameInFlightFence;
}

void Context::draw(const gfx::DrawMode& drawMode, std::size_t indexOffset, std::size_t indexLength) {
    MLN_TRACE_FUNC();
    MLN_TRACE_FUNC_GL();

    switch (drawMode.type) {
        case gfx::DrawModeType::Points:
            break;
        case gfx::DrawModeType::Lines:
        case gfx::DrawModeType::LineLoop:
        case gfx::DrawModeType::LineStrip:
            lineWidth = drawMode.size;
            break;
        default:
            break;
    }

    MBGL_CHECK_ERROR(glDrawElements(Enum<gfx::DrawModeType>::to(drawMode.type),
                                    static_cast<GLsizei>(indexLength),
                                    GL_UNSIGNED_SHORT,
                                    reinterpret_cast<GLvoid*>(sizeof(uint16_t) * indexOffset)));

    stats.numDrawCalls++;
    stats.totalDrawCalls++;
}

void Context::performCleanup() {
    MLN_TRACE_FUNC();
#ifndef NDEBUG
    // In debug builds, un-bind all texture units so that any incorrect use results in an
    // error rather than using whatever texture happened to have been bound previously.
    // This takes some time, however, so don't do it in release builds.
    {
        for (auto i = 0; i < gfx::MaxActiveTextureUnits; i++) {
            activeTextureUnit = i;
            texture[i] = 0;
        }

        bindVertexArray = 0;
    }
#endif

    for (auto id : abandonedPrograms) {
        if (program == id) {
            program.setDirty();
        }
        MBGL_CHECK_ERROR(glDeleteProgram(id));
    }
    abandonedPrograms.clear();

    for (auto id : abandonedShaders) {
        MBGL_CHECK_ERROR(glDeleteShader(id));
    }
    abandonedShaders.clear();

    if (!abandonedBuffers.empty()) {
        for (const auto id : abandonedBuffers) {
            if (vertexBuffer == id) {
                vertexBuffer.setDirty();
            } else if (globalVertexArrayState.indexBuffer == id) {
                globalVertexArrayState.indexBuffer.setDirty();
            }
        }
        MBGL_CHECK_ERROR(glDeleteBuffers(int(abandonedBuffers.size()), abandonedBuffers.data()));
        stats.numBuffers -= int(abandonedBuffers.size());
        abandonedBuffers.clear();
    }

    if (!abandonedTextures.empty()) {
        for (const auto id : abandonedTextures) {
            for (auto& binding : texture) {
                if (binding == id) {
                    binding.setDirty();
                }
            }
            texturePool->release(id);
        }
        abandonedTextures.clear();
    }

    if (!abandonedVertexArrays.empty()) {
        for (const auto id : abandonedVertexArrays) {
            if (bindVertexArray == id) {
                bindVertexArray.setDirty();
            }
        }
        MBGL_CHECK_ERROR(glDeleteVertexArrays(int(abandonedVertexArrays.size()), abandonedVertexArrays.data()));
        abandonedVertexArrays.clear();
    }

    if (!abandonedFramebuffers.empty()) {
        for (const auto id : abandonedFramebuffers) {
            if (bindFramebuffer == id) {
                bindFramebuffer.setDirty();
            }
        }
        MBGL_CHECK_ERROR(glDeleteFramebuffers(int(abandonedFramebuffers.size()), abandonedFramebuffers.data()));
        stats.numFrameBuffers -= int(abandonedFramebuffers.size());
        assert(stats.numFrameBuffers >= 0);
        abandonedFramebuffers.clear();
    }

    if (!abandonedRenderbuffers.empty()) {
        MBGL_CHECK_ERROR(glDeleteRenderbuffers(int(abandonedRenderbuffers.size()), abandonedRenderbuffers.data()));
        abandonedRenderbuffers.clear();
    }
}

void Context::reduceMemoryUsage() {
    MLN_TRACE_FUNC();
    MLN_TRACE_FUNC_GL();

    performCleanup();
    assert(texturePool);
    texturePool->shrink();

    // Ensure that all pending actions are executed to ensure that they happen
    // before the app goes to the background.
    MBGL_CHECK_ERROR(glFinish());
}

#if !defined(NDEBUG)
void Context::visualizeStencilBuffer() {
    throw std::runtime_error("Not yet implemented");
}

void Context::visualizeDepthBuffer([[maybe_unused]] const float depthRangeSize) {
    throw std::runtime_error("Not yet implemented");
}
#endif

void Context::clearStencilBuffer(const int32_t bits) {
    MLN_TRACE_FUNC();
    MLN_TRACE_FUNC_GL();

    MBGL_CHECK_ERROR(glClearStencil(bits));
    MBGL_CHECK_ERROR(glClear(GL_STENCIL_BUFFER_BIT));

    stats.stencilClears++;
}

Texture2DPool& Context::getTexturePool() {
    assert(texturePool);
    return *texturePool;
}

} // namespace gl
} // namespace mbgl
