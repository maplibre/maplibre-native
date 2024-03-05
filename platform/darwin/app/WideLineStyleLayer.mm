/* Metal Wide Line Custom Layer example implementation */

#import "WideLineStyleLayer.h"

#include "wideline.metal.h"
#include <simd/simd.h>

@implementation WideLineStyleLayer {
    // The render pipeline state
    id<MTLRenderPipelineState> _pipelineState;
    id<MTLDepthStencilState> _depthStencilState;
}

- (void)didMoveToMapView:(MLNMapView *)mapView {
    MLNBackendResource resource = [mapView backendResource];

    NSError *error = nil;
    id<MTLDevice> _device = resource.device;

    // Load shader from the default library
    id<MTLLibrary> library = [_device newDefaultLibrary];
    
    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertexTri_wideVecPerf"];
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragmentTri_wideVecPerf"];

    // Configure a pipeline descriptor that is used to create a pipeline state.
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"Wide Line GPU Pipeline";
    pipelineStateDescriptor.vertexFunction = vertexFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = resource.mtkView.colorPixelFormat;
    pipelineStateDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    pipelineStateDescriptor.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    
    // Enable blending
    MTLRenderPipelineColorAttachmentDescriptor *colorAttachment = pipelineStateDescriptor.colorAttachments[0];
    colorAttachment.blendingEnabled = YES;

    // Set blend factors and operations
    colorAttachment.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    colorAttachment.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    colorAttachment.rgbBlendOperation = MTLBlendOperationAdd;

    colorAttachment.sourceAlphaBlendFactor = MTLBlendFactorOne;
    colorAttachment.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    colorAttachment.alphaBlendOperation = MTLBlendOperationAdd;

    // Create pipeline state
    _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor
                                                             error:&error];
    NSAssert(_pipelineState, @"Failed to create pipeline state: %@", error);

    // Configure a depth stencil descriptor
    // Notice that we don't configure the stencilTest property, leaving stencil testing disabled
    MTLDepthStencilDescriptor *depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
    depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionAlways; // Or another value as needed
    depthStencilDescriptor.depthWriteEnabled = NO;

    // Create depth stencil state
    _depthStencilState = [_device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
    NSAssert(_depthStencilState, @"Failed to create depth stencil state");
}

- (void)drawInMapView:(MLNMapView *)mapView withContext:(MLNStyleLayerDrawingContext)context {
    // Use the supplied render command encoder to encode commands
    id<MTLRenderCommandEncoder> renderEncoder = self.renderEncoder;
    if(renderEncoder != nil)
    {
        MLNBackendResource resource = [mapView backendResource];
        id<MTLDevice> _device = resource.device;
        
        [renderEncoder setRenderPipelineState:_pipelineState];
        [renderEncoder setDepthStencilState:_depthStencilState];
        
        // Setup buffers
        using namespace WhirlyKitShader;
        
        struct VertexTriWideVecB
        {
            // x, y offset around the center
            vector_float3 screenPos;
            vector_float4 color;
            int index;
        };
        
        // create vertex buffer
        vector_float4 color {1.0f, 0, 0, .5f};
        VertexTriWideVecB vertexBuffer[12] {
            {{0, 0, 0}, color,  (0 << 16) + 0 },
            {{0, 0, 0}, color,  (1 << 16) + 0 },
            {{0, 0, 0}, color,  (2 << 16) + 0 },
            {{0, 0, 0}, color,  (3 << 16) + 0 },

            {{0, 0, 0}, color,  (4 << 16) + 1 },
            {{0, 0, 0}, color,  (5 << 16) + 1 },
            {{0, 0, 0}, color,  (6 << 16) + 1 },
            {{0, 0, 0}, color,  (7 << 16) + 1 },

            {{0, 0, 0}, color,  ( 8 << 16) + 2 },
            {{0, 0, 0}, color,  ( 9 << 16) + 2 },
            {{0, 0, 0}, color,  (10 << 16) + 2 },
            {{0, 0, 0}, color,  (11 << 16) + 2 },
        };

        // uniforms
        Uniforms uniforms {
            /*simd::float4x4 mvpMatrix; */  matrix_identity_float4x4,
            /*simd::float4x4 mvpMatrixDiff; */  {},
            /*simd::float4x4 mvMatrix; */   matrix_identity_float4x4,
            /*simd::float4x4 mvMatrixDiff; */   {},
            /*simd::float4x4 pMatrix; */    matrix_identity_float4x4,
            /*simd::float4x4 pMatrixDiff; */    {},
            /*simd::float2 frameSize; */    {
                static_cast<float>(resource.mtkView.drawableSize.width),
                static_cast<float>(resource.mtkView.drawableSize.height)
            }
        };
        
        // argument buffer. Need only wideVec only
        UniformWideVec wideVec {
            /*float w2;                  */  16.0f,
            /*float offset;              */  0.0f,
            /*float edge;                */  1.0f,
            /*float texRepeat;           */  0.0f,
            /*simd::float2 texOffset;    */  {},
            /*float miterLimit;          */  1.0f,
            /*WKSVertexLineJoinType join;*/  WKSVertexLineJoinMiter,
            /*WKSVertexLineCapType cap;  */  WKSVertexLineCapButt,
            /*bool hasExp;               */  false,
            /*float interClipLimit;      */  0.0f
        };
        
        // instance buffer. center line buffer. VertexTriWideVecInstance
        VertexTriWideVecInstance centerline[4] {
            {{ 0.5, -0.5, 0}, {0}, -1,  1},
            {{-0.5, -0.5, 0}, {0},  0,  2},
            {{ 0.0, +0.5, 0}, {0},  1,  3},
            {{ 0.0,  0.0, 0}, {0},  2, -1},
        };
        constexpr std::size_t pointCount = sizeof(centerline) / sizeof(centerline[0]);
        
        // Pass in the parameter data.
        [renderEncoder setVertexBytes: vertexBuffer
                               length: sizeof(vertexBuffer)
                               atIndex: WKSVertexBuffer
        ];

        [renderEncoder setVertexBytes: &uniforms
                               length: sizeof(uniforms)
                               atIndex: WKSVertUniformArgBuffer
        ];

        [renderEncoder setVertexBytes: &wideVec
                               length: sizeof(wideVec)
                               atIndex: WKSVertexArgBuffer
        ];
        
        [renderEncoder setVertexBytes: centerline
                               length: sizeof(centerline)
                               atIndex: WKSVertModelInstanceArgBuffer
        ];

        // Build triangle indexes
        uint16_t data[] {
            0, 3, 1,
            0, 2, 3,
            4 + 0, 4 + 3, 4 + 1,
            4 + 0, 4 + 2, 4 + 3,
            8 + 0, 8 + 3, 8 + 1,
            8 + 0, 8 + 2, 8 + 3,
        };
        
        // create index buffer
        id<MTLBuffer> indexBuffer = [_device newBufferWithBytes:data length: sizeof(data) options:MTLResourceStorageModeShared];
        
        // Draw the wide vectors
        [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:6*3 indexType:MTLIndexTypeUInt16 indexBuffer:indexBuffer indexBufferOffset:0 instanceCount:pointCount];
    } else {
        NSAssert(renderEncoder, @"Render encoder object is null");
    }
}

- (void)willMoveFromMapView:(MLNMapView *)mapView {
    // Clean up
}

@end
