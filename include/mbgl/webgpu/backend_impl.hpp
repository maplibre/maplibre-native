#pragma once

#include <memory>
#include <cstdint>

// When using Dawn, include its headers instead of defining our own types
#ifdef MLN_USE_DAWN
#include <webgpu/webgpu.h>
#else

// Check if WebGPU headers are already included (Dawn or wgpu)
#if !defined(WEBGPU_H_) && !defined(DAWN_WEBGPU_H)
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
typedef struct WGPUPipelineLayoutImpl* WGPUPipelineLayout;
typedef struct WGPUQuerySetImpl* WGPUQuerySet;
typedef struct WGPUQueueImpl* WGPUQueue;
typedef struct WGPURenderBundleImpl* WGPURenderBundle;
typedef struct WGPURenderBundleEncoderImpl* WGPURenderBundleEncoder;
typedef struct WGPURenderPipelineImpl* WGPURenderPipeline;
typedef struct WGPUSamplerImpl* WGPUSampler;
typedef struct WGPUShaderModuleImpl* WGPUShaderModule;
typedef struct WGPUSurfaceImpl* WGPUSurface;
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

typedef enum WGPUBufferUsage {
    WGPUBufferUsage_None = 0,
    WGPUBufferUsage_MapRead = 1,
    WGPUBufferUsage_MapWrite = 2,
    WGPUBufferUsage_CopySrc = 4,
    WGPUBufferUsage_CopyDst = 8,
    WGPUBufferUsage_Index = 16,
    WGPUBufferUsage_Vertex = 32,
    WGPUBufferUsage_Uniform = 64,
    WGPUBufferUsage_Storage = 128,
    WGPUBufferUsage_Indirect = 256,
    WGPUBufferUsage_QueryResolve = 512,
} WGPUBufferUsage;

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

typedef enum WGPUErrorType {
    WGPUErrorType_NoError = 0,
    WGPUErrorType_Validation = 1,
    WGPUErrorType_OutOfMemory = 2,
    WGPUErrorType_Unknown = 3,
    WGPUErrorType_DeviceLost = 4,
} WGPUErrorType;

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

typedef enum WGPUMapMode {
    WGPUMapMode_None = 0,
    WGPUMapMode_Read = 1,
    WGPUMapMode_Write = 2,
} WGPUMapMode;

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

typedef enum WGPUStencilOperation {
    WGPUStencilOperation_Keep = 0,
    WGPUStencilOperation_Zero = 1,
    WGPUStencilOperation_Replace = 2,
    WGPUStencilOperation_Invert = 3,
    WGPUStencilOperation_IncrementClamp = 4,
    WGPUStencilOperation_DecrementClamp = 5,
    WGPUStencilOperation_IncrementWrap = 6,
    WGPUStencilOperation_DecrementWrap = 7,
} WGPUStencilOperation;

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
    WGPUTextureFormat_RG11B10Ufloat = 26,
    WGPUTextureFormat_RGB9E5Ufloat = 27,
    WGPUTextureFormat_RG32Float = 28,
    WGPUTextureFormat_RG32Uint = 29,
    WGPUTextureFormat_RG32Sint = 30,
    WGPUTextureFormat_RGBA16Uint = 31,
    WGPUTextureFormat_RGBA16Sint = 32,
    WGPUTextureFormat_RGBA16Float = 33,
    WGPUTextureFormat_RGBA32Float = 34,
    WGPUTextureFormat_RGBA32Uint = 35,
    WGPUTextureFormat_RGBA32Sint = 36,
    WGPUTextureFormat_Depth16Unorm = 37,
    WGPUTextureFormat_Depth24Plus = 38,
    WGPUTextureFormat_Depth24PlusStencil8 = 39,
    WGPUTextureFormat_Depth32Float = 40,
    WGPUTextureFormat_Stencil8 = 41,
    WGPUTextureFormat_BC1RGBAUnorm = 42,
    WGPUTextureFormat_BC1RGBAUnormSrgb = 43,
    WGPUTextureFormat_BC2RGBAUnorm = 44,
    WGPUTextureFormat_BC2RGBAUnormSrgb = 45,
    WGPUTextureFormat_BC3RGBAUnorm = 46,
    WGPUTextureFormat_BC3RGBAUnormSrgb = 47,
    WGPUTextureFormat_BC4RUnorm = 48,
    WGPUTextureFormat_BC4RSnorm = 49,
    WGPUTextureFormat_BC5RGUnorm = 50,
    WGPUTextureFormat_BC5RGSnorm = 51,
    WGPUTextureFormat_BC6HRGBUfloat = 52,
    WGPUTextureFormat_BC6HRGBFloat = 53,
    WGPUTextureFormat_BC7RGBAUnorm = 54,
    WGPUTextureFormat_BC7RGBAUnormSrgb = 55,
} WGPUTextureFormat;

typedef enum WGPUTextureUsage {
    WGPUTextureUsage_None = 0,
    WGPUTextureUsage_CopySrc = 1,
    WGPUTextureUsage_CopyDst = 2,
    WGPUTextureUsage_TextureBinding = 4,
    WGPUTextureUsage_StorageBinding = 8,
    WGPUTextureUsage_RenderAttachment = 16,
} WGPUTextureUsage;

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

typedef enum WGPURequestAdapterStatus {
    WGPURequestAdapterStatus_Success = 0,
    WGPURequestAdapterStatus_Unavailable = 1,
    WGPURequestAdapterStatus_Error = 2,
    WGPURequestAdapterStatus_Unknown = 3,
} WGPURequestAdapterStatus;

// Callbacks
typedef void (*WGPUErrorCallback)(WGPUErrorType type, const char* message, void* userdata);
typedef void (*WGPULoggingCallback)(const char* message, void* userdata);

// Structures
struct WGPUChainedStruct {
    struct WGPUChainedStruct const* next;
    uint32_t sType;
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
    WGPUChainedStruct const* nextInChain;
    char const* label;
    uint64_t size;
    WGPUBufferUsage usage;
    bool mappedAtCreation;
};

struct WGPUCommandEncoderDescriptor {
    WGPUChainedStruct const* nextInChain;
    char const* label;
};

struct WGPUCommandBufferDescriptor {
    WGPUChainedStruct const* nextInChain;
    char const* label;
};

struct WGPUCompilationMessage {
    WGPUChainedStruct const* nextInChain;
    char const* message;
    uint64_t lineNum;
    uint64_t linePos;
};

struct WGPUConstantEntry {
    WGPUChainedStruct const* nextInChain;
    char const* key;
    double value;
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
    WGPUChainedStruct const* nextInChain;
    char const* label;
    size_t colorAttachmentCount;
    WGPURenderPassColorAttachment const* colorAttachments;
    WGPURenderPassDepthStencilAttachment const* depthStencilAttachment;
    WGPUQuerySet occlusionQuerySet;
};

struct WGPUSamplerDescriptor {
    WGPUChainedStruct const* nextInChain;
    char const* label;
    WGPUAddressMode addressModeU;
    WGPUAddressMode addressModeV;
    WGPUAddressMode addressModeW;
    WGPUFilterMode magFilter;
    WGPUFilterMode minFilter;
    WGPUFilterMode mipmapFilter;
    float lodMinClamp;
    float lodMaxClamp;
    WGPUCompareFunction compare;
    uint16_t maxAnisotropy;
};

struct WGPUShaderModuleDescriptor {
    WGPUChainedStruct const* nextInChain;
    char const* label;
};

struct WGPUShaderModuleWGSLDescriptor {
    WGPUChainedStruct chain;
    char const* code;
};

struct WGPUStencilFaceState {
    WGPUCompareFunction compare;
    WGPUStencilOperation failOp;
    WGPUStencilOperation depthFailOp;
    WGPUStencilOperation passOp;
};

struct WGPUTextureDescriptor {
    WGPUChainedStruct const* nextInChain;
    char const* label;
    WGPUTextureUsage usage;
    WGPUTextureDimension dimension;
    WGPUExtent3D size;
    WGPUTextureFormat format;
    uint32_t mipLevelCount;
    uint32_t sampleCount;
    size_t viewFormatCount;
    WGPUTextureFormat const* viewFormats;
};

struct WGPUTextureViewDescriptor {
    WGPUChainedStruct const* nextInChain;
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
    size_t attributeCount;
    WGPUVertexAttribute const* attributes;
};

struct WGPUVertexState {
    WGPUChainedStruct const* nextInChain;
    WGPUShaderModule module;
    char const* entryPoint;
    size_t constantCount;
    WGPUConstantEntry const* constants;
    size_t bufferCount;
    WGPUVertexBufferLayout const* buffers;
};

struct WGPUPrimitiveState {
    WGPUChainedStruct const* nextInChain;
    WGPUPrimitiveTopology topology;
    WGPUIndexFormat stripIndexFormat;
    WGPUFrontFace frontFace;
    WGPUCullMode cullMode;
};

struct WGPUDepthStencilState {
    WGPUChainedStruct const* nextInChain;
    WGPUTextureFormat format;
    bool depthWriteEnabled;
    WGPUCompareFunction depthCompare;
    WGPUStencilFaceState stencilFront;
    WGPUStencilFaceState stencilBack;
    uint32_t stencilReadMask;
    uint32_t stencilWriteMask;
    int32_t depthBias;
    float depthBiasSlopeScale;
    float depthBiasClamp;
};

struct WGPUMultisampleState {
    WGPUChainedStruct const* nextInChain;
    uint32_t count;
    uint32_t mask;
    bool alphaToCoverageEnabled;
};

struct WGPUColorTargetState {
    WGPUChainedStruct const* nextInChain;
    WGPUTextureFormat format;
    WGPUBlendState const* blend;
    uint32_t writeMask;
};

struct WGPUFragmentState {
    WGPUChainedStruct const* nextInChain;
    WGPUShaderModule module;
    char const* entryPoint;
    size_t constantCount;
    WGPUConstantEntry const* constants;
    size_t targetCount;
    WGPUColorTargetState const* targets;
};

struct WGPURenderPipelineDescriptor {
    WGPUChainedStruct const* nextInChain;
    char const* label;
    WGPUPipelineLayout layout;
    WGPUVertexState vertex;
    WGPUPrimitiveState primitive;
    WGPUDepthStencilState const* depthStencil;
    WGPUMultisampleState multisample;
    WGPUFragmentState const* fragment;
};

struct WGPUBindGroupEntry {
    WGPUChainedStruct const* nextInChain;
    uint32_t binding;
    WGPUBuffer buffer;
    uint64_t offset;
    uint64_t size;
    WGPUSampler sampler;
    WGPUTextureView textureView;
};

struct WGPUBindGroupDescriptor {
    WGPUChainedStruct const* nextInChain;
    char const* label;
    WGPUBindGroupLayout layout;
    size_t entryCount;
    WGPUBindGroupEntry const* entries;
};

struct WGPUBindingResource {
    WGPUChainedStruct const* nextInChain;
    WGPUBuffer buffer;
    WGPUSampler sampler;
    WGPUTextureView textureView;
};

struct WGPUBufferBinding {
    WGPUChainedStruct const* nextInChain;
    WGPUBuffer buffer;
    uint64_t offset;
    uint64_t size;
};

struct WGPUSamplerBindingLayout {
    WGPUChainedStruct const* nextInChain;
    WGPUSamplerBindingType type;
};

struct WGPUTextureBindingLayout {
    WGPUChainedStruct const* nextInChain;
    WGPUTextureSampleType sampleType;
    WGPUTextureViewDimension viewDimension;
    bool multisampled;
};

struct WGPUBindGroupLayoutEntry {
    WGPUChainedStruct const* nextInChain;
    uint32_t binding;
    uint32_t visibility;
    WGPUBufferBindingLayout buffer;
    WGPUSamplerBindingLayout sampler;
    WGPUTextureBindingLayout texture;
    WGPUStorageTextureBindingLayout storageTexture;
};

struct WGPUBindGroupLayoutDescriptor {
    WGPUChainedStruct const* nextInChain;
    char const* label;
    uint32_t entryCount;
    WGPUBindGroupLayoutEntry const* entries;
};

#endif // !defined(WEBGPU_H_) && !defined(DAWN_WEBGPU_H)
#endif // MLN_USE_DAWN

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