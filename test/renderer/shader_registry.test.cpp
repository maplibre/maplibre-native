#include <mbgl/test/util.hpp>

#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/gfx/shader.hpp>

#include <string_view>
#include <iostream>
#include <utility>

#include <mbgl/storage/resource_options.hpp>
#include <mbgl/map/map_snapshotter.hpp>
#include <mbgl/style/layers/fill_layer.hpp>
#include <mbgl/style/image_impl.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/util/color.hpp>
#include <mbgl/storage/main_resource_loader.hpp>
#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/test/map_adapter.hpp>
#include <mbgl/programs/fill_program.hpp>

using namespace mbgl;

namespace {

class MapInstance {
public:
    class ShaderAndStyleObserver : public MapObserver {
    public:
    public:
        void onDidFinishLoadingStyle() override { styleLoaded(); }

        void onRegisterShaders(gfx::ShaderRegistry& registry) override { registerShaders(registry); };

        std::function<void()> styleLoaded;
        std::function<void(gfx::ShaderRegistry&)> registerShaders;
    };

    MapInstance(float pixelRatio, MapObserver& observer)
        : frontend(pixelRatio),
          adapter(frontend,
                  observer,
                  std::make_shared<MainResourceLoader>(
                      ResourceOptions().withCachePath(":memory:").withAssetPath("test/fixtures/api/assets"),
                      ClientOptions()),
                  MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()).withPixelRatio(pixelRatio)) {}

public:
    HeadlessFrontend frontend;
    MapAdapter adapter;
};

template <uint32_t token_value>
class StubProgramBase : public gfx::Shader {
public:
    static constexpr auto Token = token_value;
    virtual uint32_t draw() { return token; }

protected:
    uint32_t token{token_value};
};

class StubProgram_1 final : public StubProgramBase<10> {
public:
    static constexpr std::string_view Name{"StubProgram_1"};
    const std::string_view typeName() const noexcept override { return Name; }

    void setToken(uint32_t tok) { token = tok; }
};

class StubProgram_2 final : public StubProgramBase<20> {
public:
    static constexpr std::string_view Name{"StubProgram_2"};
    const std::string_view typeName() const noexcept override { return Name; }
};

class StubShaderConsumer {
public:
    template <typename T>
    uint32_t useShader(gfx::ShaderRegistry& registry) {
        auto program = registry.get<T>();
        return program ? program->draw() : 0;
    }
};

} // namespace

// Ensure we can register a gfx::Shader-based type with a registry object
TEST(ShaderRegistry, RegisterShader) {
    gfx::ShaderRegistry registry;

    // The registry should start empty
    ASSERT_FALSE(registry.isShader(std::string{StubProgram_1::Name}));

    // Register the program
    auto program = std::make_shared<StubProgram_1>();
    ASSERT_TRUE(registry.registerShader(program));
    // We can't re-register the same program name
    ASSERT_FALSE(registry.registerShader(program));
    // Ensure the shader is present in the registry now
    ASSERT_TRUE(registry.isShader(std::string{StubProgram_1::Name}));
    // And we can fetch it
    ASSERT_EQ(registry.get<StubProgram_1>(), program);

    // Make sure downcasting to program1 works as expected
    StubShaderConsumer consumer;
    ASSERT_EQ(consumer.useShader<StubProgram_1>(registry), 10);
}

// Validate replacing a shader by name with a different instance
TEST(ShaderRegistry, ReplaceShaderType) {
    gfx::ShaderRegistry registry;

    ASSERT_FALSE(registry.isShader(std::string{StubProgram_1::Name}));

    auto program = std::make_shared<StubProgram_1>();
    ASSERT_TRUE(registry.registerShader(program));
    // Ensure the shader is present in the registry now
    ASSERT_TRUE(registry.isShader(std::string{StubProgram_1::Name}));
    ASSERT_TRUE(registry.isShader(std::string{program->typeName()}));

    // Make sure downcasting to program1 works as expected
    StubShaderConsumer consumer;
    ASSERT_EQ(consumer.useShader<StubProgram_1>(registry), 10);

    // StubProgram_1 and StubProgram_2 are disconnected types,
    // and as such this replacement should fail as StubProgram_2
    // has never been registered.
    auto program2 = std::make_shared<StubProgram_2>();
    ASSERT_FALSE(registry.replaceShader(program2));

    // Make a second instance of program1 and change the token
    auto program3 = std::make_shared<StubProgram_1>();
    program3->setToken(30);
    // Replace it in the registry
    ASSERT_TRUE(registry.replaceShader(program3));
    // Assert the new program downcasts from the registry as expected
    ASSERT_EQ(consumer.useShader<StubProgram_1>(registry), 30);
}

// Validate shader type info and downcasting are functioning
TEST(ShaderRegistry, ShaderRTTI) {
    auto program1 = StubProgram_1();
    auto asBase = static_cast<gfx::Shader*>(&program1);

    // Should convert to program 1 but not program 2
    ASSERT_EQ(asBase->to<StubProgram_1>(), &program1);
    ASSERT_EQ(asBase->to<StubProgram_2>(), nullptr);
}

// Register the same type under different names
TEST(ShaderRegistry, MultiRegister) {
    gfx::ShaderRegistry registry;

    ASSERT_TRUE(registry.registerShader(std::make_shared<StubProgram_1>()));
    ASSERT_TRUE(registry.registerShader(std::make_shared<StubProgram_1>(), "SecondProgram"));

    // Default option, register as the type name
    ASSERT_NE(registry.get<StubProgram_1>(), nullptr);
    // Register with an explicit name
    ASSERT_NE(registry.get<StubProgram_1>("SecondProgram"), nullptr);
    ASSERT_NE(registry.get<StubProgram_1>(), registry.get<StubProgram_1>("SecondProgram"));
}

// Test fetching
TEST(ShaderRegistry, RegistryFetch) {
    gfx::ShaderRegistry registry;

    ASSERT_TRUE(registry.registerShader(std::make_shared<StubProgram_1>()));
    ASSERT_TRUE(registry.registerShader(std::make_shared<StubProgram_1>(), "SecondProgram"));

    std::shared_ptr<StubProgram_1> progA;
    std::shared_ptr<StubProgram_1> progB;

    ASSERT_TRUE(registry.populate(progA));
    ASSERT_TRUE(registry.populate(progB, "SecondProgram"));
    ASSERT_NE(progA, progB);
    ASSERT_NE(progA, nullptr);
    ASSERT_NE(progB, nullptr);
}

// Replace a manually named shader
TEST(ShaderRegistry, NamedReplace) {
    gfx::ShaderRegistry registry;

    // Register
    ASSERT_TRUE(registry.registerShader(std::make_shared<StubProgram_1>(), "CustomName"));

    std::shared_ptr<StubProgram_1> progA;
    ASSERT_TRUE(registry.populate(progA, "CustomName"));
    ASSERT_NE(progA, nullptr);

    // Replace it with a new instance
    ASSERT_TRUE(registry.replaceShader(std::make_shared<StubProgram_1>(), "CustomName"));

    std::shared_ptr<StubProgram_1> progB;
    ASSERT_TRUE(registry.populate(progB, "CustomName"));
    ASSERT_NE(progB, nullptr);

    // Should be different instances
    ASSERT_NE(progA, progB);
}

// Test replacing an actual program instance with a similar instance
TEST(ShaderRegistry, GLSLReplacement_NoOp) {
    MapInstance::ShaderAndStyleObserver observer;
    util::RunLoop runLoop;
    auto map = MapInstance(1.0f, observer);

    // Just replace with a default instance
    observer.registerShaders = [&](gfx::ShaderRegistry& registry) {
        if (!registry.replaceShader(std::make_shared<FillProgram>(ProgramParameters(1.0f, false)))) {
            throw std::runtime_error("Failed to register shader!");
        }
    };

    observer.styleLoaded = [&] {
        auto fillLayer = std::make_unique<style::FillLayer>("green_fill", "mapbox");
        fillLayer->setSourceLayer("water");
        fillLayer->setFillColor(Color(0.25f, 0.88f, 0.82f, 1.0f));
        map.adapter.getStyle().addLayer(std::move(fillLayer));
    };

    map.adapter.getStyle().loadJSON(util::read_file("test/fixtures/api/water.json"));
    auto img = map.frontend.render(map.adapter).image;
    test::checkImage("test/fixtures/shader_registry/glsl_replace_noop", img, 0.005, 0.1);
}

// Test replacing an actual program with a similar instance using a different
// fragment shader
TEST(ShaderRegistry, GLSLReplacement1) {
    MapInstance::ShaderAndStyleObserver observer;
    util::RunLoop runLoop;
    auto map = MapInstance(1.0f, observer);

    // Replace with an instance that only renders blue
    observer.registerShaders = [&](gfx::ShaderRegistry& registry) {
        if (!registry.replaceShader(std::make_shared<FillProgram>(
                ProgramParameters(1.0f, false)
                    .withShaderSource(ProgramParameters::ProgramSource(gfx::Backend::Type::OpenGL,
                                                                       "",
                                                                       R"(
void main() {
    fragColor = vec4(0.0, 0.0, 1.0, 1.0);
}
                    )"))))) {
            throw std::runtime_error("Failed to register shader!");
        }
    };

    observer.styleLoaded = [&] {
        auto fillLayer = std::make_unique<style::FillLayer>("green_fill", "mapbox");
        fillLayer->setSourceLayer("water");
        fillLayer->setFillColor(Color(0.25f, 0.88f, 0.82f, 1.0f));
        map.adapter.getStyle().addLayer(std::move(fillLayer));
    };

    map.adapter.getStyle().loadJSON(util::read_file("test/fixtures/api/water.json"));
    auto img = map.frontend.render(map.adapter).image;
    test::checkImage("test/fixtures/shader_registry/glsl_replace_1", img, 0.005, 0.1);
}

// Test replacing an actual program with a similar instance using a different
// fragment shader
TEST(ShaderRegistry, GLSLReplacement2) {
    MapInstance::ShaderAndStyleObserver observer;
    util::RunLoop runLoop;
    auto map = MapInstance(1.0f, observer);

    // Replace with an instance that adds some red and green
    observer.registerShaders = [&](gfx::ShaderRegistry& registry) {
        if (!registry.replaceShader(std::make_shared<FillProgram>(
                ProgramParameters(1.0f, false)
                    .withShaderSource(ProgramParameters::ProgramSource(gfx::Backend::Type::OpenGL,
                                                                       "",
                                                                       R"(
#ifndef HAS_UNIFORM_u_color
varying highp vec4 color;
#else
uniform highp vec4 u_color;
#endif

#ifndef HAS_UNIFORM_u_opacity
varying lowp float opacity;
#else
uniform lowp float u_opacity;
#endif

void main() {
#ifdef HAS_UNIFORM_u_color
    highp vec4 color = u_color;
#endif
#ifdef HAS_UNIFORM_u_opacity
    lowp float opacity = u_opacity;
#endif

    fragColor = mix(color * opacity,
        vec4(0.3, 0.5, 0.0, 1.0), 0.5);

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
                    )"))))) {
            throw std::runtime_error("Failed to register shader!");
        }
    };

    observer.styleLoaded = [&] {
        auto fillLayer = std::make_unique<style::FillLayer>("green_fill", "mapbox");
        fillLayer->setSourceLayer("water");
        fillLayer->setFillColor(Color(0.25f, 0.88f, 0.82f, 1.0f));
        map.adapter.getStyle().addLayer(std::move(fillLayer));
    };

    map.adapter.getStyle().loadJSON(util::read_file("test/fixtures/api/water.json"));
    auto img = map.frontend.render(map.adapter).image;
    test::checkImage("test/fixtures/shader_registry/glsl_replace_2", img, 0.005, 0.1);
}
