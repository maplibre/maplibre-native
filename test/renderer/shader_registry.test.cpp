#include <mbgl/test/util.hpp>

#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/gfx/shader.hpp>

#include <string_view>
#include <iostream>

using namespace mbgl;

namespace {

template<uint32_t token_value>
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
        const std::string_view name() const noexcept override {
            return Name;
        }

        void setToken(uint32_t tok) { token = tok; }
};

class StubProgram_2 final : public StubProgramBase<20> {
    public:
        static constexpr std::string_view Name{"StubProgram_2"};
        const std::string_view name() const noexcept override {
            return Name;
        }
};

class StubShaderConsumer {
    public:
        template<typename T>
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
    ASSERT_TRUE(registry.isShader(std::string{program->name()}));

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
