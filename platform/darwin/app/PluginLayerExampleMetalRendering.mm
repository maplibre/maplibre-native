/*
 PluginLayerExampleMetalRendering

 This is a port lf the CustomStyleLayerExample into the plug-in architecture.   It assumes
 that the rendering is being done in metal since it's added to the core via a darwin method

 */

#import "PluginLayerExampleMetalRendering.h"
#import <MetalKit/MetalKit.h>
#import "Mapbox.h"

typedef struct
{
    vector_float2 position;
    vector_float4 color;
} Vertex;

@interface PluginLayerExampleMetalRendering () {
    // The render pipeline state
    id<MTLRenderPipelineState> _pipelineState;
    id<MTLDepthStencilState> _depthStencilStateWithoutStencil;

    // Properties
    float _offsetX;
    float _offsetY;
    float _scale;
    float _r, _g, _b, _a;


}

@end


@implementation PluginLayerExampleMetalRendering


// This is the layer type in the style that is used
+(MLNPluginLayerCapabilities *)layerCapabilities {

    MLNPluginLayerCapabilities *tempResult = [[MLNPluginLayerCapabilities alloc] init];
    tempResult.layerID = @"plugin-layer-metal-rendering";
    tempResult.tileKind = MLNPluginLayerTileKindNotRequired;
    tempResult.requiresPass3D = YES;

    // Setup the properties
    tempResult.layerProperties = @[
        // The scale property
        [MLNPluginLayerProperty propertyWithName:@"scale"
                                    propertyType:MLNPluginLayerPropertyTypeSingleFloat
                                    defaultValue:@(1.0)],

        // The scale property
        [MLNPluginLayerProperty propertyWithName:@"fill-color"
                                    propertyType:MLNPluginLayerPropertyTypeColor
                                    defaultValue:[UIColor blueColor]]

        // TBD
    ];

    return tempResult;

}

- (void)createShaders:(MLNMapView *)mapView {
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




// The overrides
-(void)onRenderLayer:(MLNMapView *)mapView
       renderEncoder:(id<MTLRenderCommandEncoder>)renderEncoder {

    MLNBackendResource* resource = [mapView backendResource];

    if (_pipelineState == nil) {
        [self createShaders:mapView];
        if (_scale == 0) {
            _scale = 1;
        }
        _r = 0;
        _g = 0;
        _b = 0;
        _a = 1;
    }


    if (renderEncoder == nil) {
        return;
    }


    vector_uint2 _viewportSize;
    _viewportSize.x = resource.mtkView.drawableSize.width;
    _viewportSize.y = resource.mtkView.drawableSize.height;


    Vertex triangleVerticesWithColor[] = {
        // 2D positions,    RGBA colors
        { {  (250 + _offsetX) * _scale,  (-250 + _offsetY) * _scale }, { _r, _g, _b, _a } },
        { { (-250 + _offsetX) * _scale,  (-250 + _offsetY) * _scale }, { _r, _g, _b, _a} },
        { {    (0 + _offsetX) * _scale,   (250 + _offsetY) * _scale }, { _r, _g, _b, _a } },
    };

//    Vertex triangleVertices[] = {
//        // 2D positions,    RGBA colors
//        { {  (250 + _offsetX) * _scale,  (-250 + _offsetY) * _scale }, { 1, 0, 0, 1 } },
//        { { (-250 + _offsetX) * _scale,  (-250 + _offsetY) * _scale }, { 0, 1, 0, 1} },
//        { {    (0 + _offsetX) * _scale,   (250 + _offsetY) * _scale }, { 0, 0, 1, 1 } },
//    };

    [renderEncoder setRenderPipelineState:_pipelineState];
    [renderEncoder setDepthStencilState:_depthStencilStateWithoutStencil];

    // Pass in the parameter data.
    [renderEncoder setVertexBytes:triangleVerticesWithColor
                           length:sizeof(triangleVerticesWithColor)
                          atIndex:0];

    [renderEncoder setVertexBytes:&_viewportSize
                           length:sizeof(_viewportSize)
                           atIndex:1];

    // Draw the triangle.
    [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
                      vertexStart:0
                      vertexCount:3];
}

-(void)onUpdateLayer:(MLNPluginLayerDrawingContext)drawingContext {
    // Update any metadata needed for the frame
    //NSLog(@"onUpdateLayer");
}

-(void)onUpdateLayerProperties:(NSDictionary *)layerProperties {
    //NSLog(@"Metal Layer Rendering Properties: %@", layerProperties);

    NSNumber *offsetX = [layerProperties objectForKey:@"offset-x"];
    if (offsetX) {
        _offsetX = [[layerProperties objectForKey:@"offset-x"] floatValue];
    }

    NSNumber *offsetY = [layerProperties objectForKey:@"offset-y"];
    if (offsetY) {
        _offsetY = [[layerProperties objectForKey:@"offset-y"] floatValue];
    }

    NSNumber *scale = [layerProperties objectForKey:@"scale"];
    if (scale) {
        if ([scale isKindOfClass:[NSNumber class]]) {
            _scale = [scale floatValue];
        }
    }

    NSString *fillColor = [layerProperties objectForKey:@"fill-color"];
    if (fillColor) {
        //NSLog(@"Fill Color: %@", fillColor);
        fillColor = [fillColor stringByReplacingOccurrencesOfString:@"rgba(" withString:@""];
        fillColor = [fillColor stringByReplacingOccurrencesOfString:@")" withString:@""];
        NSArray *components = [fillColor componentsSeparatedByString:@","];
        if ([components count] == 4) {
            _r = [[components objectAtIndex:0] floatValue] / 255.0;
            _g = [[components objectAtIndex:1] floatValue] / 255.0;
            _b = [[components objectAtIndex:2] floatValue] / 255.0;
            _a = [[components objectAtIndex:3] floatValue];
        }
        
    }

}

@end
