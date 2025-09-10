#pragma once

#include <memory>
#include <cstdint>

// Check if WebGPU headers are already included (Dawn or wgpu)
#ifndef WEBGPU_H_
// WebGPU C API types
// These are the standard WebGPU types that both Dawn and wgpu implement
// Only define these if the actual WebGPU headers are not available

// Basic types
typedef struct WGPUAdapterImpl* WGPUAdapter;
typedef struct WGPUBindGroupImpl* WGPUBindGroup;
typedef struct WGPUBindGroupLayoutImpl* WGPUBindGroupLayout;
typedef struct WGPUBufferImpl* WGPUBuffer;
typedef struct WGPUInstanceImpl* WGPUInstance;
typedef struct WGPUSwapChainImpl* WGPUSwapChain;
typedef struct WGPURenderPassEncoderImpl* WGPURenderPassEncoder;
typedef struct WGPUCommandBufferImpl* WGPUCommandBuffer;
typedef struct WGPUCommandEncoderImpl* WGPUCommandEncoder;
typedef struct WGPUComputePassEncoderImpl* WGPUComputePassEncoder;
typedef struct WGPUComputePipelineImpl* WGPUComputePipeline;
typedef struct WGPUDeviceImpl* WGPUDevice;
typedef struct WGPUInstanceImpl* WGPUInstance;
typedef struct WGPUPipelineLayoutImpl* WGPUPipelineLayout;
typedef struct WGPUQuerySetImpl* WGPUQuerySet;
typedef struct WGPUQueueImpl* WGPUQueue;
typedef struct WGPURenderBundleImpl* WGPURenderBundle;
typedef struct WGPURenderBundleEncoderImpl* WGPURenderBundleEncoder;
typedef struct WGPURenderPassEncoderImpl* WGPURenderPassEncoder;
typedef struct WGPURenderPipelineImpl* WGPURenderPipeline;
typedef struct WGPUSamplerImpl* WGPUSampler;
typedef struct WGPUShaderModuleImpl* WGPUShaderModule;
typedef struct WGPUSurfaceImpl* WGPUSurface;
typedef struct WGPUSwapChainImpl* WGPUSwapChain;
typedef struct WGPUTextureImpl* WGPUTexture;
typedef struct WGPUTextureViewImpl* WGPUTextureView;

// Enums
typedef enum WGPUAddressMode {
    WGPUAddressMode_Repeat = 0,
    WGPUAddressMode_MirrorRepeat = 1,
    WGPUAddressMode_ClampToEdge = 2,
} WGPUAddressMode;

typedef enum WGPUBackendType {
    WGPUBackendType_Null = 0,
    WGPUBackendType_WebGPU = 1,
    WGPUBackendType_D3D11 = 2,
    WGPUBackendType_D3D12 = 3,
    WGPUBackendType_Metal = 4,
    WGPUBackendType_Vulkan = 5,
    WGPUBackendType_OpenGL = 6,
    WGPUBackendType_OpenGLES = 7,
} WGPUBackendType;

typedef enum WGPUBlendFactor {
    WGPUBlendFactor_Zero = 0,
    WGPUBlendFactor_One = 1,
    WGPUBlendFactor_Src = 2,
    WGPUBlendFactor_OneMinusSrc = 3,
    WGPUBlendFactor_SrcAlpha = 4,
    WGPUBlendFactor_OneMinusSrcAlpha = 5,
    WGPUBlendFactor_Dst = 6,
    WGPUBlendFactor_OneMinusDst = 7,
    WGPUBlendFactor_DstAlpha = 8,
    WGPUBlendFactor_OneMinusDstAlpha = 9,
    WGPUBlendFactor_SrcAlphaSaturated = 10,
    WGPUBlendFactor_Constant = 11,
    WGPUBlendFactor_OneMinusConstant = 12,
} WGPUBlendFactor;

typedef enum WGPUBlendOperation {
    WGPUBlendOperation_Add = 0,
    WGPUBlendOperation_Subtract = 1,
    WGPUBlendOperation_ReverseSubtract = 2,
    WGPUBlendOperation_Min = 3,
    WGPUBlendOperation_Max = 4,
} WGPUBlendOperation;

typedef enum WGPUBufferBindingType {
    WGPUBufferBindingType_Undefined = 0,
    WGPUBufferBindingType_Uniform = 1,
    WGPUBufferBindingType_Storage = 2,
    WGPUBufferBindingType_ReadOnlyStorage = 3,
} WGPUBufferBindingType;

typedef uint32_t WGPUBufferUsage;
enum WGPUBufferUsage_ {
    WGPUBufferUsage_None = 0x00000000,
    WGPUBufferUsage_MapRead = 0x00000001,
    WGPUBufferUsage_MapWrite = 0x00000002,
    WGPUBufferUsage_CopySrc = 0x00000004,
    WGPUBufferUsage_CopyDst = 0x00000008,
    WGPUBufferUsage_Index = 0x00000010,
    WGPUBufferUsage_Vertex = 0x00000020,
    WGPUBufferUsage_Uniform = 0x00000040,
    WGPUBufferUsage_Storage = 0x00000080,
    WGPUBufferUsage_Indirect = 0x00000100,
    WGPUBufferUsage_QueryResolve = 0x00000200,
};

typedef uint32_t WGPUColorWriteMask;
enum WGPUColorWriteMask_ {
    WGPUColorWriteMask_None = 0x00000000,
    WGPUColorWriteMask_Red = 0x00000001,
    WGPUColorWriteMask_Green = 0x00000002,
    WGPUColorWriteMask_Blue = 0x00000004,
    WGPUColorWriteMask_Alpha = 0x00000008,
    WGPUColorWriteMask_All = 0x0000000F,
};

typedef enum WGPUCompareFunction {
    WGPUCompareFunction_Undefined = 0,
    WGPUCompareFunction_Never = 1,
    WGPUCompareFunction_Less = 2,
    WGPUCompareFunction_LessEqual = 3,
    WGPUCompareFunction_Greater = 4,
    WGPUCompareFunction_GreaterEqual = 5,
    WGPUCompareFunction_Equal = 6,
    WGPUCompareFunction_NotEqual = 7,
    WGPUCompareFunction_Always = 8,
} WGPUCompareFunction;

typedef enum WGPUCullMode {
    WGPUCullMode_None = 0,
    WGPUCullMode_Front = 1,
    WGPUCullMode_Back = 2,
} WGPUCullMode;

typedef enum WGPUFilterMode {
    WGPUFilterMode_Nearest = 0,
    WGPUFilterMode_Linear = 1,
} WGPUFilterMode;

typedef enum WGPUFrontFace {
    WGPUFrontFace_CCW = 0,
    WGPUFrontFace_CW = 1,
} WGPUFrontFace;

typedef enum WGPUIndexFormat {
    WGPUIndexFormat_Undefined = 0,
    WGPUIndexFormat_Uint16 = 1,
    WGPUIndexFormat_Uint32 = 2,
} WGPUIndexFormat;

typedef enum WGPULoadOp {
    WGPULoadOp_Undefined = 0,
    WGPULoadOp_Clear = 1,
    WGPULoadOp_Load = 2,
} WGPULoadOp;

typedef enum WGPUMipmapFilterMode {
    WGPUMipmapFilterMode_Nearest = 0,
    WGPUMipmapFilterMode_Linear = 1,
} WGPUMipmapFilterMode;

typedef enum WGPUPowerPreference {
    WGPUPowerPreference_Undefined = 0,
    WGPUPowerPreference_LowPower = 1,
    WGPUPowerPreference_HighPerformance = 2,
} WGPUPowerPreference;

typedef enum WGPUPresentMode {
    WGPUPresentMode_Immediate = 0,
    WGPUPresentMode_Mailbox = 1,
    WGPUPresentMode_Fifo = 2,
} WGPUPresentMode;

typedef enum WGPUPrimitiveTopology {
    WGPUPrimitiveTopology_PointList = 0,
    WGPUPrimitiveTopology_LineList = 1,
    WGPUPrimitiveTopology_LineStrip = 2,
    WGPUPrimitiveTopology_TriangleList = 3,
    WGPUPrimitiveTopology_TriangleStrip = 4,
} WGPUPrimitiveTopology;

typedef enum WGPUSamplerBindingType {
    WGPUSamplerBindingType_Undefined = 0,
    WGPUSamplerBindingType_Filtering = 1,
    WGPUSamplerBindingType_NonFiltering = 2,
    WGPUSamplerBindingType_Comparison = 3,
} WGPUSamplerBindingType;

typedef uint32_t WGPUShaderStage;
enum WGPUShaderStage_ {
    WGPUShaderStage_None = 0x00000000,
    WGPUShaderStage_Vertex = 0x00000001,
    WGPUShaderStage_Fragment = 0x00000002,
    WGPUShaderStage_Compute = 0x00000004,
};

typedef enum WGPUSType {
    WGPUSType_Invalid = 0x00000000,
    WGPUSType_ShaderModuleWGSLDescriptor = 0x00000006,
    WGPUSType_SurfaceDescriptorFromMetalLayer = 0x00000007,
    WGPUSType_SurfaceDescriptorFromWindowsHWND = 0x00000008,
    WGPUSType_SurfaceDescriptorFromXlibWindow = 0x00000009,
} WGPUSType;

typedef uint32_t WGPUMapMode;
enum WGPUMapMode_ {
    WGPUMapMode_None = 0x00000000,
    WGPUMapMode_Read = 0x00000001,
    WGPUMapMode_Write = 0x00000002,
};

typedef enum WGPUBufferMapAsyncStatus {
    WGPUBufferMapAsyncStatus_Success = 0,
    WGPUBufferMapAsyncStatus_Error = 1,
    WGPUBufferMapAsyncStatus_UnmappedBeforeCallback = 2,
    WGPUBufferMapAsyncStatus_DestroyedBeforeCallback = 3,
    WGPUBufferMapAsyncStatus_DeviceLost = 4,
} WGPUBufferMapAsyncStatus;

typedef enum WGPUStoreOp {
    WGPUStoreOp_Undefined = 0,
    WGPUStoreOp_Store = 1,
    WGPUStoreOp_Discard = 2,
} WGPUStoreOp;

typedef enum WGPUTextureAspect {
    WGPUTextureAspect_All = 0,
    WGPUTextureAspect_StencilOnly = 1,
    WGPUTextureAspect_DepthOnly = 2,
} WGPUTextureAspect;

typedef enum WGPUTextureDimension {
    WGPUTextureDimension_1D = 0,
    WGPUTextureDimension_2D = 1,
    WGPUTextureDimension_3D = 2,
} WGPUTextureDimension;

typedef enum WGPUTextureFormat {
    WGPUTextureFormat_Undefined = 0,
    WGPUTextureFormat_R8Unorm = 1,
    WGPUTextureFormat_R8Snorm = 2,
    WGPUTextureFormat_R8Uint = 3,
    WGPUTextureFormat_R8Sint = 4,
    WGPUTextureFormat_R16Uint = 5,
    WGPUTextureFormat_R16Sint = 6,
    WGPUTextureFormat_R16Float = 7,
    WGPUTextureFormat_RG8Unorm = 8,
    WGPUTextureFormat_RG8Snorm = 9,
    WGPUTextureFormat_RG8Uint = 10,
    WGPUTextureFormat_RG8Sint = 11,
    WGPUTextureFormat_R32Float = 12,
    WGPUTextureFormat_R32Uint = 13,
    WGPUTextureFormat_R32Sint = 14,
    WGPUTextureFormat_RG16Uint = 15,
    WGPUTextureFormat_RG16Sint = 16,
    WGPUTextureFormat_RG16Float = 17,
    WGPUTextureFormat_RGBA8Unorm = 18,
    WGPUTextureFormat_RGBA8UnormSrgb = 19,
    WGPUTextureFormat_RGBA8Snorm = 20,
    WGPUTextureFormat_RGBA8Uint = 21,
    WGPUTextureFormat_RGBA8Sint = 22,
    WGPUTextureFormat_BGRA8Unorm = 23,
    WGPUTextureFormat_BGRA8UnormSrgb = 24,
    WGPUTextureFormat_RGB10A2Unorm = 25,
    WGPUTextureFormat_RG32Float = 26,
    WGPUTextureFormat_RG32Uint = 27,
    WGPUTextureFormat_RG32Sint = 28,
    WGPUTextureFormat_RGBA16Uint = 29,
    WGPUTextureFormat_RGBA16Sint = 30,
    WGPUTextureFormat_RGBA16Float = 31,
    WGPUTextureFormat_RGBA32Float = 32,
    WGPUTextureFormat_RGBA32Uint = 33,
    WGPUTextureFormat_RGBA32Sint = 34,
    WGPUTextureFormat_Depth32Float = 35,
    WGPUTextureFormat_Depth24Plus = 36,
    WGPUTextureFormat_Depth24PlusStencil8 = 37,
} WGPUTextureFormat;

typedef enum WGPUTextureSampleType {
    WGPUTextureSampleType_Undefined = 0,
    WGPUTextureSampleType_Float = 1,
    WGPUTextureSampleType_UnfilterableFloat = 2,
    WGPUTextureSampleType_Depth = 3,
    WGPUTextureSampleType_Sint = 4,
    WGPUTextureSampleType_Uint = 5,
} WGPUTextureSampleType;

typedef uint32_t WGPUTextureUsage;
enum WGPUTextureUsage_ {
    WGPUTextureUsage_None = 0x00000000,
    WGPUTextureUsage_CopySrc = 0x00000001,
    WGPUTextureUsage_CopyDst = 0x00000002,
    WGPUTextureUsage_TextureBinding = 0x00000004,
    WGPUTextureUsage_StorageBinding = 0x00000008,
    WGPUTextureUsage_RenderAttachment = 0x00000010,
};

typedef enum WGPUTextureViewDimension {
    WGPUTextureViewDimension_Undefined = 0,
    WGPUTextureViewDimension_1D = 1,
    WGPUTextureViewDimension_2D = 2,
    WGPUTextureViewDimension_2DArray = 3,
    WGPUTextureViewDimension_Cube = 4,
    WGPUTextureViewDimension_CubeArray = 5,
    WGPUTextureViewDimension_3D = 6,
} WGPUTextureViewDimension;

typedef enum WGPUVertexFormat {
    WGPUVertexFormat_Undefined = 0,
    WGPUVertexFormat_Uint8x2 = 1,
    WGPUVertexFormat_Uint8x4 = 2,
    WGPUVertexFormat_Sint8x2 = 3,
    WGPUVertexFormat_Sint8x4 = 4,
    WGPUVertexFormat_Unorm8x2 = 5,
    WGPUVertexFormat_Unorm8x4 = 6,
    WGPUVertexFormat_Snorm8x2 = 7,
    WGPUVertexFormat_Snorm8x4 = 8,
    WGPUVertexFormat_Uint16x2 = 9,
    WGPUVertexFormat_Uint16x4 = 10,
    WGPUVertexFormat_Sint16x2 = 11,
    WGPUVertexFormat_Sint16x4 = 12,
    WGPUVertexFormat_Unorm16x2 = 13,
    WGPUVertexFormat_Unorm16x4 = 14,
    WGPUVertexFormat_Snorm16x2 = 15,
    WGPUVertexFormat_Snorm16x4 = 16,
    WGPUVertexFormat_Float16x2 = 17,
    WGPUVertexFormat_Float16x4 = 18,
    WGPUVertexFormat_Float32 = 19,
    WGPUVertexFormat_Float32x2 = 20,
    WGPUVertexFormat_Float32x3 = 21,
    WGPUVertexFormat_Float32x4 = 22,
    WGPUVertexFormat_Uint32 = 23,
    WGPUVertexFormat_Uint32x2 = 24,
    WGPUVertexFormat_Uint32x3 = 25,
    WGPUVertexFormat_Uint32x4 = 26,
    WGPUVertexFormat_Sint32 = 27,
    WGPUVertexFormat_Sint32x2 = 28,
    WGPUVertexFormat_Sint32x3 = 29,
    WGPUVertexFormat_Sint32x4 = 30,
} WGPUVertexFormat;

typedef enum WGPUVertexStepMode {
    WGPUVertexStepMode_Vertex = 0,
    WGPUVertexStepMode_Instance = 1,
} WGPUVertexStepMode;

// Structures
struct WGPUChainedStruct {
    struct WGPUChainedStruct const* next;
    WGPUSType sType;
};

struct WGPUColor {
    double r;
    double g;
    double b;
    double a;
};

struct WGPUExtent3D {
    uint32_t width;
    uint32_t height;
    uint32_t depthOrArrayLayers;
};

struct WGPUOrigin3D {
    uint32_t x;
    uint32_t y;
    uint32_t z;
};

// Descriptor structures
struct WGPUBindGroupEntry {
    uint32_t binding;
    WGPUBuffer buffer;
    uint64_t offset;
    uint64_t size;
    WGPUSampler sampler;
    WGPUTextureView textureView;
};

struct WGPUBindGroupLayoutEntry {
    uint32_t binding;
    WGPUShaderStage visibility;
    struct {
        WGPUBufferBindingType type;
        bool hasDynamicOffset;
        uint64_t minBindingSize;
    } buffer;
    struct {
        WGPUSamplerBindingType type;
    } sampler;
    struct {
        WGPUTextureSampleType sampleType;
        WGPUTextureViewDimension viewDimension;
        bool multisampled;
    } texture;
    struct {
        WGPUTextureFormat format;
    } storageTexture;
};

struct WGPUBlendComponent {
    WGPUBlendOperation operation;
    WGPUBlendFactor srcFactor;
    WGPUBlendFactor dstFactor;
};

struct WGPUBlendState {
    WGPUBlendComponent color;
    WGPUBlendComponent alpha;
};

struct WGPUBufferDescriptor {
    struct WGPUChainedStruct const* nextInChain;
    char const* label;
    uint64_t size;
    WGPUBufferUsage usage;
    bool mappedAtCreation;
};

struct WGPUColorTargetState {
    struct WGPUChainedStruct const* nextInChain;
    WGPUTextureFormat format;
    WGPUBlendState const* blend;
    WGPUColorWriteMask writeMask;
};

struct WGPUCommandBufferDescriptor {
    struct WGPUChainedStruct const* nextInChain;
    char const* label;
};

struct WGPUCommandEncoderDescriptor {
    struct WGPUChainedStruct const* nextInChain;
    char const* label;
};

struct WGPUDepthStencilState {
    struct WGPUChainedStruct const* nextInChain;
    WGPUTextureFormat format;
    bool depthWriteEnabled;
    WGPUCompareFunction depthCompare;
    struct {
        WGPUCompareFunction compare;
        uint32_t failOp;
        uint32_t depthFailOp;
        uint32_t passOp;
    } stencilFront;
    struct {
        WGPUCompareFunction compare;
        uint32_t failOp;
        uint32_t depthFailOp;
        uint32_t passOp;
    } stencilBack;
    uint32_t stencilReadMask;
    uint32_t stencilWriteMask;
    int32_t depthBias;
    float depthBiasSlopeScale;
    float depthBiasClamp;
};

struct WGPUFragmentState {
    struct WGPUChainedStruct const* nextInChain;
    WGPUShaderModule module;
    char const* entryPoint;
    uint32_t targetCount;
    WGPUColorTargetState const* targets;
};

struct WGPUImageCopyBuffer {
    struct WGPUChainedStruct const* nextInChain;
    WGPUBuffer buffer;
    struct {
        uint64_t offset;
        uint32_t bytesPerRow;
        uint32_t rowsPerImage;
    } layout;
};

struct WGPUImageCopyTexture {
    struct WGPUChainedStruct const* nextInChain;
    WGPUTexture texture;
    uint32_t mipLevel;
    WGPUOrigin3D origin;
    WGPUTextureAspect aspect;
};

struct WGPUInstanceDescriptor {
    struct WGPUChainedStruct const* nextInChain;
};

struct WGPUMultisampleState {
    struct WGPUChainedStruct const* nextInChain;
    uint32_t count;
    uint32_t mask;
    bool alphaToCoverageEnabled;
};

struct WGPUPipelineLayoutDescriptor {
    struct WGPUChainedStruct const* nextInChain;
    char const* label;
    uint32_t bindGroupLayoutCount;
    WGPUBindGroupLayout const* bindGroupLayouts;
};

struct WGPUPrimitiveState {
    struct WGPUChainedStruct const* nextInChain;
    WGPUPrimitiveTopology topology;
    WGPUIndexFormat stripIndexFormat;
    WGPUFrontFace frontFace;
    WGPUCullMode cullMode;
};

struct WGPURenderPassColorAttachment {
    WGPUTextureView view;
    WGPUTextureView resolveTarget;
    WGPULoadOp loadOp;
    WGPUStoreOp storeOp;
    WGPUColor clearValue;
};

struct WGPURenderPassDepthStencilAttachment {
    WGPUTextureView view;
    WGPULoadOp depthLoadOp;
    WGPUStoreOp depthStoreOp;
    float depthClearValue;
    bool depthReadOnly;
    WGPULoadOp stencilLoadOp;
    WGPUStoreOp stencilStoreOp;
    uint32_t stencilClearValue;
    bool stencilReadOnly;
};

struct WGPURenderPassDescriptor {
    struct WGPUChainedStruct const* nextInChain;
    char const* label;
    uint32_t colorAttachmentCount;
    WGPURenderPassColorAttachment const* colorAttachments;
    WGPURenderPassDepthStencilAttachment const* depthStencilAttachment;
};

struct WGPURenderPipelineDescriptor {
    struct WGPUChainedStruct const* nextInChain;
    char const* label;
    WGPUPipelineLayout layout;
    struct {
        struct WGPUChainedStruct const* nextInChain;
        WGPUShaderModule module;
        char const* entryPoint;
        uint32_t bufferCount;
        struct WGPUVertexBufferLayout const* buffers;
    } vertex;
    WGPUPrimitiveState primitive;
    WGPUDepthStencilState const* depthStencil;
    WGPUMultisampleState multisample;
    WGPUFragmentState const* fragment;
};

struct WGPUSamplerDescriptor {
    struct WGPUChainedStruct const* nextInChain;
    char const* label;
    WGPUAddressMode addressModeU;
    WGPUAddressMode addressModeV;
    WGPUAddressMode addressModeW;
    WGPUFilterMode magFilter;
    WGPUFilterMode minFilter;
    WGPUMipmapFilterMode mipmapFilter;
    float lodMinClamp;
    float lodMaxClamp;
    WGPUCompareFunction compare;
    uint16_t maxAnisotropy;
};

struct WGPUShaderModuleDescriptor {
    struct WGPUChainedStruct const* nextInChain;
    char const* label;
};

struct WGPUShaderModuleWGSLDescriptor {
    WGPUChainedStruct chain;
    char const* code;
};

struct WGPUSurfaceDescriptorFromMetalLayer {
    WGPUChainedStruct chain;
    void* layer;
};

struct WGPUSurfaceDescriptorFromWindowsHWND {
    WGPUChainedStruct chain;
    void* hinstance;
    void* hwnd;
};

struct WGPUSurfaceDescriptorFromXlibWindow {
    WGPUChainedStruct chain;
    void* display;
    uint32_t window;
};

struct WGPURequestAdapterOptions {
    struct WGPUChainedStruct const* nextInChain;
    WGPUSurface compatibleSurface;
    WGPUPowerPreference powerPreference;
    bool forceFallbackAdapter;
};

struct WGPUSwapChainDescriptor {
    struct WGPUChainedStruct const* nextInChain;
    char const* label;
    WGPUTextureUsage usage;
    WGPUTextureFormat format;
    uint32_t width;
    uint32_t height;
    WGPUPresentMode presentMode;
};

struct WGPUTextureDataLayout {
    struct WGPUChainedStruct const* nextInChain;
    uint64_t offset;
    uint32_t bytesPerRow;
    uint32_t rowsPerImage;
};

struct WGPUTextureDescriptor {
    struct WGPUChainedStruct const* nextInChain;
    char const* label;
    WGPUTextureUsage usage;
    WGPUTextureDimension dimension;
    WGPUExtent3D size;
    WGPUTextureFormat format;
    uint32_t mipLevelCount;
    uint32_t sampleCount;
    uint32_t viewFormatCount;
    WGPUTextureFormat const* viewFormats;
};

struct WGPUTextureViewDescriptor {
    struct WGPUChainedStruct const* nextInChain;
    char const* label;
    WGPUTextureFormat format;
    WGPUTextureViewDimension dimension;
    uint32_t baseMipLevel;
    uint32_t mipLevelCount;
    uint32_t baseArrayLayer;
    uint32_t arrayLayerCount;
    WGPUTextureAspect aspect;
};

struct WGPUVertexAttribute {
    WGPUVertexFormat format;
    uint64_t offset;
    uint32_t shaderLocation;
};

struct WGPUVertexBufferLayout {
    uint64_t arrayStride;
    WGPUVertexStepMode stepMode;
    uint32_t attributeCount;
    WGPUVertexAttribute const* attributes;
};

struct WGPUBindGroupDescriptor {
    struct WGPUChainedStruct const* nextInChain;
    char const* label;
    WGPUBindGroupLayout layout;
    uint32_t entryCount;
    WGPUBindGroupEntry const* entries;
};

struct WGPUBindGroupLayoutDescriptor {
    struct WGPUChainedStruct const* nextInChain;
    char const* label;
    uint32_t entryCount;
    WGPUBindGroupLayoutEntry const* entries;
};

#endif // WEBGPU_H_

namespace mbgl {
namespace webgpu {

// Abstract interface for WebGPU backend implementation
// This allows us to support both Dawn and wgpu without tying the code to either
class BackendImpl {
public:
    virtual ~BackendImpl() = default;

    virtual WGPUInstance getInstance() const = 0;
    virtual WGPUAdapter getAdapter() const = 0;
    virtual WGPUDevice getDevice() const = 0;
    virtual WGPUQueue getQueue() const = 0;
    virtual WGPUSurface getSurface() const = 0;
    
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    
    // Factory method to create the appropriate implementation
    static std::unique_ptr<BackendImpl> create();
};

} // namespace webgpu
} // namespace mbgl

// WebGPU API function declarations
// These are placeholder implementations - in a real implementation, these would
// call into Dawn or wgpu-native libraries

inline WGPUBuffer wgpuDeviceCreateBuffer(WGPUDevice /*device*/, const WGPUBufferDescriptor* /*descriptor*/) {
    return nullptr;
}

inline void* wgpuBufferGetMappedRange(WGPUBuffer /*buffer*/, size_t /*offset*/, size_t /*size*/) {
    return nullptr;
}

inline void* wgpuBufferGetConstMappedRange(WGPUBuffer /*buffer*/, size_t /*offset*/, size_t /*size*/) {
    return nullptr;
}

inline void wgpuBufferUnmap(WGPUBuffer /*buffer*/) {
}

inline void wgpuBufferRelease(WGPUBuffer /*buffer*/) {
}

inline WGPUCommandEncoder wgpuDeviceCreateCommandEncoder(WGPUDevice /*device*/, const WGPUCommandEncoderDescriptor* /*descriptor*/) {
    return nullptr;
}

inline void wgpuCommandEncoderCopyBufferToBuffer(WGPUCommandEncoder /*encoder*/, WGPUBuffer /*source*/, uint64_t /*sourceOffset*/, WGPUBuffer /*destination*/, uint64_t /*destinationOffset*/, uint64_t /*size*/) {
}

inline WGPUCommandBuffer wgpuCommandEncoderFinish(WGPUCommandEncoder /*encoder*/, const WGPUCommandBufferDescriptor* /*descriptor*/) {
    return nullptr;
}

inline void wgpuQueueSubmit(WGPUQueue /*queue*/, size_t /*commandCount*/, const WGPUCommandBuffer* /*commands*/) {
}

inline void wgpuCommandBufferRelease(WGPUCommandBuffer /*commandBuffer*/) {
}

inline void wgpuCommandEncoderRelease(WGPUCommandEncoder /*encoder*/) {
}

inline void wgpuQueueWriteBuffer(WGPUQueue /*queue*/, WGPUBuffer /*buffer*/, uint64_t /*bufferOffset*/, const void* /*data*/, size_t /*size*/) {
}

inline void wgpuQueueWriteTexture(WGPUQueue /*queue*/, const WGPUImageCopyTexture* /*destination*/, const void* /*data*/, size_t /*dataSize*/, const WGPUTextureDataLayout* /*dataLayout*/, const WGPUExtent3D* /*writeSize*/) {
}

inline WGPUTexture wgpuDeviceCreateTexture(WGPUDevice /*device*/, const WGPUTextureDescriptor* /*descriptor*/) {
    return nullptr;
}

inline WGPUTextureView wgpuTextureCreateView(WGPUTexture /*texture*/, const WGPUTextureViewDescriptor* /*descriptor*/) {
    return nullptr;
}

inline void wgpuTextureRelease(WGPUTexture /*texture*/) {
}

inline void wgpuTextureViewRelease(WGPUTextureView /*textureView*/) {
}

inline WGPUSampler wgpuDeviceCreateSampler(WGPUDevice /*device*/, const WGPUSamplerDescriptor* /*descriptor*/) {
    return nullptr;
}

inline void wgpuSamplerRelease(WGPUSampler /*sampler*/) {
}

inline WGPUShaderModule wgpuDeviceCreateShaderModule(WGPUDevice /*device*/, const WGPUShaderModuleDescriptor* /*descriptor*/) {
    return nullptr;
}

inline void wgpuShaderModuleRelease(WGPUShaderModule /*shaderModule*/) {
}

inline WGPUBindGroupLayout wgpuDeviceCreateBindGroupLayout(WGPUDevice /*device*/, const WGPUBindGroupLayoutDescriptor* /*descriptor*/) {
    return nullptr;
}

inline void wgpuBindGroupLayoutRelease(WGPUBindGroupLayout /*bindGroupLayout*/) {
}

inline WGPUPipelineLayout wgpuDeviceCreatePipelineLayout(WGPUDevice /*device*/, const WGPUPipelineLayoutDescriptor* /*descriptor*/) {
    return nullptr;
}

inline void wgpuPipelineLayoutRelease(WGPUPipelineLayout /*pipelineLayout*/) {
}

inline WGPURenderPipeline wgpuDeviceCreateRenderPipeline(WGPUDevice /*device*/, const WGPURenderPipelineDescriptor* /*descriptor*/) {
    return nullptr;
}

inline void wgpuRenderPipelineRelease(WGPURenderPipeline /*pipeline*/) {
}

inline void wgpuCommandEncoderPushDebugGroup(WGPUCommandEncoder /*encoder*/, const char* /*groupLabel*/) {
}

inline void wgpuCommandEncoderPopDebugGroup(WGPUCommandEncoder /*encoder*/) {
}