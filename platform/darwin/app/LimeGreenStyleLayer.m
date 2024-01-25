#if !MLN_RENDER_BACKEND_METAL

/* OPENGL Custom Layer example implementation */
#import "LimeGreenStyleLayer.h"
#import <GLKit/GLKit.h>

@implementation LimeGreenStyleLayer {
    GLuint _program;
    GLuint _vertexShader;
    GLuint _fragmentShader;
    GLuint _buffer;
    GLuint _aPos;
}

- (void)didMoveToMapView:(MLNMapView *)mapView {
    static const GLchar *vertexShaderSource = "#version 300 es\nlayout (location = 0) in vec2 a_pos; void main() { gl_Position = vec4(a_pos, 1, 1); }";
    static const GLchar *fragmentShaderSource = "#version 300 es\nout highp vec4 fragColor; void main() { fragColor = vec4(0, 0.5, 0, 0.5); }";

    _program = glCreateProgram();
    _vertexShader = glCreateShader(GL_VERTEX_SHADER);
    _fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(_vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(_vertexShader);
    glAttachShader(_program, _vertexShader);
    glShaderSource(_fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(_fragmentShader);
    glAttachShader(_program, _fragmentShader);
    glLinkProgram(_program);
    _aPos = glGetAttribLocation(_program, "a_pos");

    GLfloat triangle[] = { 0, 0.5, 0.5, -0.5, -0.5, -0.5 };
    glGenBuffers(1, &_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, _buffer);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), triangle, GL_STATIC_DRAW);
}

- (void)drawInMapView:(MLNMapView *)mapView withContext:(MLNStyleLayerDrawingContext)context {
    glUseProgram(_program);
    glBindBuffer(GL_ARRAY_BUFFER, _buffer);
    glEnableVertexAttribArray(_aPos);
    glVertexAttribPointer(_aPos, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
}

- (void)willMoveFromMapView:(MLNMapView *)mapView {
    if (!_program) {
        return;
    }

    glDeleteBuffers(1, &_buffer);
    glDetachShader(_program, _vertexShader);
    glDetachShader(_program, _fragmentShader);
    glDeleteShader(_vertexShader);
    glDeleteShader(_fragmentShader);
    glDeleteProgram(_program);
}

@end

#else // MLN_RENDER_BACKEND_METAL:

/* Metal Custom Layer example implementation */
#import "LimeGreenStyleLayer.h"

@implementation LimeGreenStyleLayer {
    id<MTLDevice> _device;

    // The render pipeline generated from the vertex and fragment shaders in the .metal shader file.
    id<MTLRenderPipelineState> _pipelineState;
    
    // The command queue used to pass commands to the device.
    id<MTLCommandQueue> _commandQueue;

    // The current size of the view, used as an input to the vertex shader.
    vector_uint2 _viewportSize;
}

- (void)didMoveToMapView:(MLNMapView *)mapView {
    // TODO: get device
    // TODO: custom setup
    MLNBackendResource res = [mapView backendResource];
    
    NSString *shaderSource = @
"    #include <metal_stdlib>\n"
"    using namespace metal;\n"
"    typedef enum AAPLVertexInputIndex\n"
"    {\n"
"        AAPLVertexInputIndexVertices     = 0,\n"
"        AAPLVertexInputIndexViewportSize = 1,\n"
"    } AAPLVertexInputIndex;\n"
"    typedef struct\n"
"    {\n"
"        vector_float2 position;\n"
"        vector_float4 color;\n"
"    } AAPLVertex;\n"
"    struct RasterizerData\n"
"    {\n"
"        float4 position [[position]];\n"
"        float4 color;\n"
"    };\n"
"    vertex RasterizerData\n"
"    vertexShader(uint vertexID [[vertex_id]],\n"
"                 constant AAPLVertex *vertices [[buffer(AAPLVertexInputIndexVertices)]],\n"
"                 constant vector_uint2 *viewportSizePointer [[buffer(AAPLVertexInputIndexViewportSize)]])\n"
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

    
    NSError *error = nil;
    _device = res.device;
    id<MTLLibrary> library = [_device newLibraryWithSource:shaderSource options:nil error:&error];
    NSAssert(library, @"Error compiling shaders: %@", error);
    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertexShader"];
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragmentShader"];

    // Configure a pipeline descriptor that is used to create a pipeline state.
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"Simple Pipeline";
    pipelineStateDescriptor.vertexFunction = vertexFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = res.mtkView.colorPixelFormat;
    pipelineStateDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    pipelineStateDescriptor.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    
    _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor
                                                             error:&error];
    NSAssert(_pipelineState, @"Failed to create pipeline state: %@", error);
    
    // Create the command queue
    _commandQueue = [_device newCommandQueue];
}

- (void)drawInMapView:(MLNMapView *)mapView withContext:(MLNStyleLayerDrawingContext)context {
    // TODO: encode commands
    MLNBackendResource res = [mapView backendResource];
    
    typedef enum AAPLVertexInputIndex
    {
        AAPLVertexInputIndexVertices     = 0,
        AAPLVertexInputIndexViewportSize = 1,
    } AAPLVertexInputIndex;

    CGSize size = res.mtkView.drawableSize;
    _viewportSize.x = size.width;
    _viewportSize.y = size.height;
    
    typedef struct
    {
        vector_float2 position;
        vector_float4 color;
    } AAPLVertex;

    static const AAPLVertex triangleVertices[] =
    {
        // 2D positions,    RGBA colors
        { {  250,  -250 }, { 1, 0, 0, 1 } },
        { { -250,  -250 }, { 0, 1, 0, 1 } },
        { {    0,   250 }, { 0, 0, 1, 1 } },
    };
    
    // Create a new command buffer for each render pass to the current drawable.
//    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
//    commandBuffer.label = @"MyCommand";
    
    // Obtain a renderPassDescriptor generated from the view's drawable textures.
//    MTLRenderPassDescriptor *renderPassDescriptor = res.mtkView.currentRenderPassDescriptor;
    
    if(context.renderEncoder != nil)
    {
        // load framebuffer
//        MTLLoadAction previousLoadAction = renderPassDescriptor.colorAttachments[0].loadAction;
//        renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;

        // Use the supplied render command encoder
        id<MTLRenderCommandEncoder> renderEncoder = context.renderEncoder;

        // Create a render command encoder.
//        id<MTLRenderCommandEncoder> renderEncoder =
//        [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
//        renderEncoder.label = @"MyRenderEncoder";
        
//        // Set the region of the drawable to draw into.
//        [renderEncoder setViewport:(MTLViewport){0.0, 0.0, _viewportSize.x, _viewportSize.y, 0.0, 1.0 }];
        
        [renderEncoder setRenderPipelineState:_pipelineState];
        
        // Pass in the parameter data.
        [renderEncoder setVertexBytes:triangleVertices
                               length:sizeof(triangleVertices)
                              atIndex:AAPLVertexInputIndexVertices];
        
        [renderEncoder setVertexBytes:&_viewportSize
                               length:sizeof(_viewportSize)
                              atIndex:AAPLVertexInputIndexViewportSize];
        
        // Draw the triangle.
        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
                          vertexStart:0
                          vertexCount:3];
        
//        [renderEncoder endEncoding];
        
        // Schedule a present once the framebuffer is complete using the current drawable.
//        [commandBuffer presentDrawable:res.mtkView.currentDrawable];
    }
    
    // Finalize rendering here & push the command buffer to the GPU.
//    [commandBuffer commit];
}

- (void)willMoveFromMapView:(MLNMapView *)mapView {
    // TODO: clean up
}

@end

#endif
