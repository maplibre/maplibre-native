#if !MLN_RENDER_BACKEND_METAL

/* OPENGL Custom Layer example implementation */
#import "CustomStyleLayerExample.h"
#import <GLKit/GLKit.h>

@implementation CustomStyleLayerExample {
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
#import "CustomStyleLayerExample.h"

@implementation CustomStyleLayerExample {
    // The render pipeline state
    id<MTLRenderPipelineState> _pipelineState;
    id<MTLDepthStencilState> _depthStencilStateWithoutStencil;
}

- (void)didMoveToMapView:(MLNMapView *)mapView {
    MLNBackendResource* resource = [mapView backendResource];

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


    NSError *error = nil;
    id<MTLDevice> _device = resource.device;
    id<MTLLibrary> library = [_device newLibraryWithSource:shaderSource options:nil error:&error];
    NSAssert(library, @"Error compiling shaders: %@", error);
    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertexShader"];
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragmentShader"];

    // Configure a pipeline descriptor that is used to create a pipeline state.
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"Simple Pipeline";
    pipelineStateDescriptor.vertexFunction = vertexFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = resource.mtkView.colorPixelFormat;
    pipelineStateDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    pipelineStateDescriptor.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;

    _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor
                                                             error:&error];
    NSAssert(_pipelineState, @"Failed to create pipeline state: %@", error);

    // Notice that we don't configure the stencilTest property, leaving stencil testing disabled
    MTLDepthStencilDescriptor *depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
    depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionAlways; // Or another value as needed
    depthStencilDescriptor.depthWriteEnabled = NO;

    _depthStencilStateWithoutStencil = [_device newDepthStencilStateWithDescriptor:depthStencilDescriptor];

}

- (void)drawInMapView:(MLNMapView *)mapView withContext:(MLNStyleLayerDrawingContext)context {
    // Use the supplied render command encoder to encode commands
    id<MTLRenderCommandEncoder> renderEncoder = self.renderEncoder;
    if(renderEncoder != nil)
    {
        MLNBackendResource* resource = [mapView backendResource];

        vector_uint2 _viewportSize;
        _viewportSize.x = resource.mtkView.drawableSize.width;
        _viewportSize.y = resource.mtkView.drawableSize.height;

        typedef struct
        {
            vector_float2 position;
            vector_float4 color;
        } Vertex;

        static const Vertex triangleVertices[] =
        {
            // 2D positions,    RGBA colors
            { {  250,  -250 }, { 1, 0, 0, 1 } },
            { { -250,  -250 }, { 0, 1, 0, 1 } },
            { {    0,   250 }, { 0, 0, 1, 1 } },
        };

        [renderEncoder setRenderPipelineState:_pipelineState];
        [renderEncoder setDepthStencilState:_depthStencilStateWithoutStencil];

        // Pass in the parameter data.
        [renderEncoder setVertexBytes:triangleVertices
                               length:sizeof(triangleVertices)
                               atIndex:0];

        [renderEncoder setVertexBytes:&_viewportSize
                               length:sizeof(_viewportSize)
                               atIndex:1];

        // Draw the triangle.
        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
                          vertexStart:0
                          vertexCount:3];
    }
}

- (void)willMoveFromMapView:(MLNMapView *)mapView {
    // Clean up
}

@end

#endif
