#include "PluginLayerExampleAndroidRenderingCPP.h"
#include <plugin/plugin_platform_android.hpp>

#define OPENGL

#if METAL
#include <mbgl/interface/plugin_platform_darwin.h>
#include <simd/simd.h>
#elif VULKAN

#elif defined(OPENGL)
#include <GLES3/gl3.h>
#include <android/log.h>
#define LOG_TAG "PluginLayerOpenGL"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif

#include <rapidjson/document.h>
#include <sstream>
/*
typedef struct
{
    vector_float2 position;
    vector_float4 color;
} Vertex;
*/

typedef struct
{
    float positionX;
    float positionY;
    float colorR;
    float colorG;
    float colorB;
    float colorA;
} Vertex;

using namespace app;

// Returns the layer type string that is used in the style
std::string AndroidPluginLayerType::getLayerType() {
    // Base class does nothing
    return "plugin-layer-android-rendering-cpp";
}

// If this layer type requires pass 3d
bool AndroidPluginLayerType::requiresPass3D() {
    return true;
}

// This creates the actual map layer.  Should be overridden by the
// implementor and return a class descended from the MapLayer below
std::shared_ptr<mbgl::plugin::MapLayer> AndroidPluginLayerType::createMapLayer() {

    std::shared_ptr<AndroidPluginLayer> tempResult = std::make_shared<AndroidPluginLayer>();
    return tempResult;
}

// The list of properties
std::vector<std::shared_ptr<mbgl::plugin::LayerProperty>> AndroidPluginLayerType::getLayerProperties() {

    std::vector<std::shared_ptr<mbgl::plugin::LayerProperty>> tempResult;

    {
        std::shared_ptr<mbgl::plugin::LayerProperty> lp = std::make_shared<mbgl::plugin::LayerProperty>();
        lp->propertyName = "scale";
        lp->propertyType = mbgl::plugin::LayerProperty::PropertyType::SingleFloat;
        lp->singleFloatDefaultValue = 1.0;
        tempResult.push_back(lp);
    }

    {
        std::shared_ptr<mbgl::plugin::LayerProperty> lp = std::make_shared<mbgl::plugin::LayerProperty>();
        lp->propertyName = "fill-color";
        lp->propertyType = mbgl::plugin::LayerProperty::PropertyType::Color;
        lp->hasDefaultColorValue = true;
        lp->colorDefaultValue = {0.0, 0.0, 1.0, 1.0};
        tempResult.push_back(lp);
    }

    return tempResult;

}




// Overrides
void AndroidPluginLayer::onRender(const mbgl::plugin::RenderingContext *renderingContext) {

#if METAL
    auto metalRenderingContext = static_cast<const mbgl::plugin::RenderingContextMetal *>(renderingContext);
    if (!_renderingResourcesCreated) {
        createMetalShaders(renderingContext);
    }
#elif defined(OPENGL)
    if (!_renderingResourcesCreated) {
        createOpenGLShaders();
    }
#endif

    int viewportSizeX = _lastDrawingContext.drawableSize[0];
    int viewportSizeY = _lastDrawingContext.drawableSize[1];
    //_scale = 1.0;
    _offsetX = 0;
    _offsetY = 0;
    _offsetY = _scale * 30;
    _offsetX = _scale * 30;
    float size = 100;

    Vertex triangleVerticesWithColor[] = {
            // 2D positions,    RGBA colors
            {   (size + _offsetX) * _scale,  (-size + _offsetY) * _scale ,  1, 1, 1, _a  },
            {  (-size + _offsetX) * _scale,  (-size + _offsetY) * _scale ,  1, 1, 1, _a },
            {     (0 + _offsetX) * _scale,   (size + _offsetY) * _scale ,  _r, _g, _b, _a  },
    };

#if METAL

    [metalRenderingContext->renderEncoder setRenderPipelineState:_pipelineState];
    [metalRenderingContext->renderEncoder setDepthStencilState:_depthStencilStateWithoutStencil];

    // Pass in the parameter data.
    [metalRenderingContext->renderEncoder setVertexBytes:triangleVerticesWithColor
                           length:sizeof(triangleVerticesWithColor)
                          atIndex:0];

    [metalRenderingContext->renderEncoder setVertexBytes:&viewportSize
                           length:sizeof(viewportSize)
                           atIndex:1];

    // Draw the triangle.
    [metalRenderingContext->renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
                      vertexStart:0
                      vertexCount:3];

#elif defined(OPENGL)

    // Prepare vertex data: interleaved position (vec2) and color (vec4)
    float vertices[] = {
        // Vertex 0: position, then color
        triangleVerticesWithColor[0].positionX, triangleVerticesWithColor[0].positionY,
        triangleVerticesWithColor[0].colorR, triangleVerticesWithColor[0].colorG,
        triangleVerticesWithColor[0].colorB, triangleVerticesWithColor[0].colorA,
        // Vertex 1
        triangleVerticesWithColor[1].positionX, triangleVerticesWithColor[1].positionY,
        triangleVerticesWithColor[1].colorR, triangleVerticesWithColor[1].colorG,
        triangleVerticesWithColor[1].colorB, triangleVerticesWithColor[1].colorA,
        // Vertex 2
        triangleVerticesWithColor[2].positionX, triangleVerticesWithColor[2].positionY,
        triangleVerticesWithColor[2].colorR, triangleVerticesWithColor[2].colorG,
        triangleVerticesWithColor[2].colorB, triangleVerticesWithColor[2].colorA,
    };

    // Save current OpenGL state that we'll modify
    GLboolean depthTestEnabled;
    GLint currentProgram;
    GLint currentVao;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVao);

    // Use our shader program
    glUseProgram(_shaderProgram);

    // Set the viewport size uniform
    glUniform2f(_viewportSizeLocation, (float)viewportSizeX, (float)viewportSizeY);

    // Bind our VAO
    glBindVertexArray(_vao);

    // Update the VBO with new vertex data
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    // Disable depth testing so triangle always draws (like Metal depthCompareFunction = Always)
    glDisable(GL_DEPTH_TEST);

    // Draw the triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Restore OpenGL state
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
    glBindVertexArray(currentVao);
    glUseProgram(currentProgram);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

#endif

}

void AndroidPluginLayer::onAddedToMap() {

}

void AndroidPluginLayer::onUpdate(const mbgl::plugin::DrawingContext &drawingContext) {
    _lastDrawingContext = drawingContext;
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void AndroidPluginLayer::onUpdateLayerProperties(const std::string &layerProperties) {

    using namespace rapidjson;

    Document document;
    document.Parse(layerProperties.c_str());
    if (document.HasMember("offset-x")) {
        _offsetX = document["offset-x"].GetFloat();
    }
    if (document.HasMember("offset-y")) {
        _offsetY = document["offset-y"].GetFloat();
    }
    if (document.HasMember("scale")) {
        _scale = document["scale"].GetFloat();
    }
    if (document.HasMember("fill-color")) {
        std::string fillColor = document["fill-color"].GetString();
        replace(fillColor, "rgba(", "");
        replace(fillColor, ")", "");
        std::stringstream fileColorSS(fillColor);
        std::string colorValue;
        std::getline(fileColorSS, colorValue, ',');
        _r = std::stof(colorValue) / 255.0;
        std::getline(fileColorSS, colorValue, ',');
        _g = std::stof(colorValue) / 255.0;
        std::getline(fileColorSS, colorValue, ',');
        _b = std::stof(colorValue) / 255.0;
        std::getline(fileColorSS, colorValue, ',');
        _a = std::stof(colorValue);

    }

}

void AndroidPluginLayer::onMemoryReductionEvent() {

}

#if defined(OPENGL)
void AndroidPluginLayer::createOpenGLShaders() {
    // Vertex shader - transforms pixel coordinates to normalized device coordinates
    const char* vertexShaderSource = R"(#version 300 es
        layout (location = 0) in vec2 aPosition;
        layout (location = 1) in vec4 aColor;

        uniform vec2 viewportSize;

        out vec4 vertColor;

        void main() {
            // Convert from pixel space to normalized device coordinates
            vec2 normalizedPos = aPosition / (viewportSize / 2.0);
            gl_Position = vec4(normalizedPos, 0.0, 1.0);
            vertColor = aColor;
        }
    )";

    // Fragment shader - outputs the interpolated color
    const char* fragmentShaderSource = R"(#version 300 es
        precision mediump float;

        in vec4 vertColor;
        out vec4 fragColor;

        void main() {
            fragColor = vertColor;
        }
    )";

    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // Check for vertex shader compile errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        LOGE("Vertex shader compilation failed: %s", infoLog);
    }

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // Check for fragment shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        LOGE("Fragment shader compilation failed: %s", infoLog);
    }

    // Link shaders into a program
    _shaderProgram = glCreateProgram();
    glAttachShader(_shaderProgram, vertexShader);
    glAttachShader(_shaderProgram, fragmentShader);
    glLinkProgram(_shaderProgram);

    // Check for linking errors
    glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(_shaderProgram, 512, nullptr, infoLog);
        LOGE("Shader program linking failed: %s", infoLog);
    }

    // Get uniform location for viewport size
    _viewportSizeLocation = glGetUniformLocation(_shaderProgram, "viewportSize");
    LOGI("Viewport size uniform location: %d", _viewportSizeLocation);

    // Clean up shaders (they're linked into the program now)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Create VAO and VBO (required for OpenGL ES 3.0)
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);

    // Set up the VAO
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    // Allocate buffer space for 3 vertices (will be updated each frame)
    // Each vertex: 2 floats position + 4 floats color = 6 floats
    glBufferData(GL_ARRAY_BUFFER, 3 * 6 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    // Set up vertex attribute pointers
    // Position: location 0, 2 floats, stride = 6 floats, offset = 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    // Color: location 1, 4 floats, stride = 6 floats, offset = 2 floats
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));

    // Unbind VAO (VBO stays associated with the VAO)
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    _renderingResourcesCreated = true;
    LOGI("OpenGL shaders and buffers created successfully. Program: %u, VAO: %u, VBO: %u",
         _shaderProgram, _vao, _vbo);
}
#endif





#if METAL
void MetalPluginLayer::createMetalShaders(const mbgl::plugin::RenderingContext *renderingContext) {

    @autoreleasepool {

        NSString *shaderSource = @
    "    #include <metal_stdlib>\n"
    "    using namespace metal;\n"
    "    typedef struct\n"
    "    {\n"
    "        vector_float2 position;\n"
    "        vector_float4 color;\n"
    "    } Vertex;\n"
    "    struct RasterizerData\n"
    "    {\n"
    "        float4 position [[position]];\n"
    "        float4 color;\n"
    "    };\n"
    "    vertex RasterizerData\n"
    "    vertexShader(uint vertexID [[vertex_id]],\n"
    "                 constant Vertex *vertices [[buffer(0)]],\n"
    "                 constant vector_uint2 *viewportSizePointer [[buffer(1)]])\n"
    "    {\n"
    "        RasterizerData out;\n"
    "        float2 pixelSpacePosition = vertices[vertexID].position.xy;\n"
    "        vector_float2 viewportSize = vector_float2(*viewportSizePointer);\n"
    "        out.position = vector_float4(0.0, 0.0, 0.0, 1.0);\n"
    "        out.position.xy = pixelSpacePosition / (viewportSize / 2.0);\n"
    "        out.color = vertices[vertexID].color;\n"
    "        return out;\n"
    "    }\n"
    "    fragment float4 fragmentShader(RasterizerData in [[stage_in]])\n"
    "    {\n"
    "        return in.color;\n"
    "    }\n";


        auto metalRenderingContext = static_cast<const mbgl::plugin::RenderingContextMetal *>(renderingContext);

        NSError *error = nil;
        id<MTLDevice> _device = metalRenderingContext->metalDevice;
        id<MTLLibrary> library = [_device newLibraryWithSource:shaderSource options:nil error:&error];
        id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertexShader"];
        id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragmentShader"];

        // Configure a pipeline descriptor that is used to create a pipeline state.
        MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineStateDescriptor.label = @"Simple Pipeline";
        pipelineStateDescriptor.vertexFunction = vertexFunction;
        pipelineStateDescriptor.fragmentFunction = fragmentFunction;
        pipelineStateDescriptor.colorAttachments[0].pixelFormat = _pixelFormat;
        pipelineStateDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
        pipelineStateDescriptor.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;

        _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor
                                                                 error:&error];

        // Notice that we don't configure the stencilTest property, leaving stencil testing disabled
        MTLDepthStencilDescriptor *depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
        depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionAlways; // Or another value as needed
        depthStencilDescriptor.depthWriteEnabled = NO;

        _depthStencilStateWithoutStencil = [_device newDepthStencilStateWithDescriptor:depthStencilDescriptor];


    }

    _renderingResourcesCreated = true;


}
#endif

#if VULKAN
/* Vertex shader
#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;

layout(binding = 0) uniform ViewportSize {
    vec2 viewportSize;
};

layout(location = 0) out vec4 vertColor;

void main()
{
    vec2 pixelSpace = inPosition;
    vec2 norm = pixelSpace / (viewportSize / 2.0);

    gl_Position = vec4(norm, 0.0, 1.0);
    vertColor = inColor;
}
*/

/* Fragment shader

 #version 450

 layout(location = 0) in vec4 vertColor;
 layout(location = 0) out vec4 outColor;

 void main()
 {
     outColor = vertColor;
 }


 */



class RenderingContextVulkan: public RenderingContext {
public:
    VkDevice vulkanDevice;
    VkRenderPass renderPass;
};

#include <shaderc/shaderc.hpp>

std::vector<uint32_t> compileGLSL_glslang(const std::string& source, EShLanguage stage)
{
    glslang::InitializeProcess();

    const char* shaderStrings[1];
    shaderStrings[0] = source.c_str();

    glslang::TShader shader(stage);
    shader.setStrings(shaderStrings, 1);

    if (!shader.parse(&glslang::DefaultTBuiltInResource, 450, false, EShMsgDefault))
        throw std::runtime_error(shader.getInfoLog());

    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(EShMsgDefault))
        throw std::runtime_error(program.getInfoLog());

    std::vector<uint32_t> spirv;
    glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

    glslang::FinalizeProcess();
    return spirv;
}

std::string vertSource = R"(
#version 450
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 0) out vec4 vertColor;
void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    vertColor = inColor;
}
)";

auto spirvCode = compileGLSLtoSPV(vertSource, shaderc_vertex_shader);

// Create a Vulkan shader module:
VkShaderModuleCreateInfo info{};
info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
info.codeSize = spirvCode.size() * sizeof(uint32_t);
info.pCode = spirvCode.data();

VkShaderModule vertModule;
vkCreateShaderModule(device, &info, nullptr, &vertModule);


void VulkanPluginLayer::createVulkanShaders(const mbgl::plugin::RenderingContext* renderingContext)
{
    auto vkCtx = static_cast<const mbgl::plugin::RenderingContextVulkan*>(renderingContext);
    VkDevice device = vkCtx->device;

    // 1. Load SPIR-V shader modules
    VkShaderModule vertModule = loadShaderModule(device, "vert.spv");
    VkShaderModule fragModule = loadShaderModule(device, "frag.spv");

    // 2. Pipeline shader stage info
    VkPipelineShaderStageCreateInfo vertStageInfo{};
    vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStageInfo.module = vertModule;
    vertStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragStageInfo{};
    fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStageInfo.module = fragModule;
    fragStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertStageInfo, fragStageInfo };

    // 3. Vertex input layout (matches MSL struct)
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(float) * 6; // vec2 pos + vec4 color
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescriptions[2]{};

    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // position
    attributeDescriptions[0].offset = 0;

    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT; // color
    attributeDescriptions[1].offset = sizeof(float) * 2;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = 2;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    // 4. Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // 5. Viewport & scissor are dynamic
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // 6. Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_NONE; // same as default Metal
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.lineWidth = 1.0f;

    // 7. Multisampling
    VkPipelineMultisampleStateCreateInfo multisample{};
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // 8. Depth/stencil (matches Metal depthWriteEnabled = NO)
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_ALWAYS;  // MetalCompareFunctionAlways
    depthStencil.stencilTestEnable = VK_FALSE;           // matches Metal (stencil disabled)

    // 9. Color blend
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
          VK_COLOR_COMPONENT_R_BIT |
          VK_COLOR_COMPONENT_G_BIT |
          VK_COLOR_COMPONENT_B_BIT |
          VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // 10. Pipeline layout (for uniform: viewport size)
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // contains viewport UBO

    vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);

    // 11. Pipeline creation
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisample;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = vkCtx->renderPass;
    pipelineInfo.subpass = 0;

    vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);

    // Cleanup modules (safe: pipeline keeps copies)
    vkDestroyShaderModule(device, vertModule, nullptr);
    vkDestroyShaderModule(device, fragModule, nullptr);

    _renderingResourcesCreated = true;
}



#endif
