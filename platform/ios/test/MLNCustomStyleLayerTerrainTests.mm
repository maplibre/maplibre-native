#import <MapLibre.h>
#import <XCTest/XCTest.h>

#import "MLNMapView_Private.h"

#include <cmath>
#include <cstring>

#include <mln/map/map.hpp>
#include <mln/style/style.hpp>
#include <mln/style/terrain.hpp>

// Records the drawing context of the first frame drawn after `drawExpectation` is set
// whose terrain exaggeration matches `expectsTerrain`.
@interface MLNTerrainRecordingStyleLayer : MLNCustomStyleLayer
@property (nonatomic) XCTestExpectation *drawExpectation;
@property (nonatomic) BOOL expectsTerrain;
@property (nonatomic) MLNStyleLayerDrawingContext drawnContext;
#if MLN_RENDER_BACKEND_METAL
@property (nonatomic) id<MTLTexture> drawnDepthTexture;
#endif
@end

@implementation MLNTerrainRecordingStyleLayer

- (void)drawInMapView:(MLNMapView *)mapView withContext:(MLNStyleLayerDrawingContext)context {
  if (!self.drawExpectation || (context.terrainExaggeration != 0) != self.expectsTerrain) {
    return;
  }
  self.drawnContext = context;
#if MLN_RENDER_BACKEND_METAL
  self.drawnDepthTexture = self.terrainDepthTexture;
#endif
  [self.drawExpectation fulfill];
  self.drawExpectation = nil;
}

@end

#if MLN_RENDER_BACKEND_METAL
// Draws two screen-filling quads with a depth test and no depth writes, one a little above the
// flat ground at sea level and one as far below it, and counts the fragments of each that pass
// the test. The mesh hides the one below; the one above is in front of it.
@interface MLNTerrainDepthProbeStyleLayer : MLNCustomStyleLayer
@property (nonatomic) NSUInteger aboveGroundFragments;
@property (nonatomic) NSUInteger belowGroundFragments;
// How many frames drawn with terrain the two counts above have been read from.
@property (nonatomic) NSUInteger countedFrames;
// How many frames the layer has been asked to draw, with terrain or not.
@property (nonatomic) NSUInteger drawnFrames;
// Seconds the layer holds the main thread for in the first frame it is asked to draw, standing in
// for a frame that is slow to render.
@property (nonatomic) NSTimeInterval firstFrameDelay;
@end

@implementation MLNTerrainDepthProbeStyleLayer {
  id<MTLRenderPipelineState> _pipeline;
  id<MTLDepthStencilState> _depthState;
}

static NSString *const MLNTerrainDepthProbeSource = @R"(
#include <metal_stdlib>
using namespace metal;

struct ProbeOut {
    float4 position [[position]];
};

vertex ProbeOut probeVertex(uint vid [[vertex_id]], constant float &depth [[buffer(0)]]) {
    const float2 corners[6] = {float2(-1, -1), float2(1, -1), float2(1, 1),
                               float2(-1, -1), float2(1, 1), float2(-1, 1)};
    ProbeOut out;
    out.position = float4(corners[vid], depth, 1.0);
    return out;
}

// The count is a side effect, so the depth test is asked for first.
[[early_fragment_tests]]
fragment float4 probeFragment(ProbeOut in [[stage_in]], constant uint &which [[buffer(0)]],
                              device atomic_uint *counts [[buffer(1)]]) {
    atomic_fetch_add_explicit(&counts[which], 1, memory_order_relaxed);
    return float4(0.0);
}
)";

// The Metal depth, in [0, 1], of a point over the map's center at a height in meters, as the
// terrain mesh's own would be: the projection is in OpenGL clip space.
static float MLNDepthOverCenter(MLNStyleLayerDrawingContext context, double meters) {
  const double worldSize = 512.0 * std::pow(2.0, context.zoomLevel);
  const double x = (context.centerCoordinate.longitude + 180.0) / 360.0 * worldSize;
  const double latitude = context.centerCoordinate.latitude * M_PI / 180.0;
  const double y = (1.0 - std::log(std::tan(M_PI / 4.0 + latitude / 2.0)) / M_PI) / 2.0 * worldSize;
  const MLNMatrix4 m = context.projectionMatrix;
  const double z = m.m02 * x + m.m12 * y + m.m22 * meters + m.m32;
  const double w = m.m03 * x + m.m13 * y + m.m23 * meters + m.m33;
  return static_cast<float>((z / w + 1.0) / 2.0);
}

- (void)drawInMapView:(MLNMapView *)mapView withContext:(MLNStyleLayerDrawingContext)context {
  if (self.drawnFrames++ == 0 && self.firstFrameDelay > 0) {
    [NSThread sleepForTimeInterval:self.firstFrameDelay];
  }
  id<MTLRenderCommandEncoder> encoder = self.renderEncoder;
  MTLRenderPassDescriptor *pass = self.renderPassDesc;
  if (!encoder || !pass.colorAttachments[0].texture || context.terrainExaggeration == 0) {
    return;
  }
  id<MTLDevice> device = encoder.device;
  if (!_pipeline) {
    NSError *error = nil;
    id<MTLLibrary> library = [device newLibraryWithSource:MLNTerrainDepthProbeSource
                                                  options:nil
                                                    error:&error];
    MTLRenderPipelineDescriptor *descriptor = [[MTLRenderPipelineDescriptor alloc] init];
    descriptor.vertexFunction = [library newFunctionWithName:@"probeVertex"];
    descriptor.fragmentFunction = [library newFunctionWithName:@"probeFragment"];
    descriptor.colorAttachments[0].pixelFormat = pass.colorAttachments[0].texture.pixelFormat;
    descriptor.colorAttachments[0].writeMask = MTLColorWriteMaskNone;
    descriptor.depthAttachmentPixelFormat = pass.depthAttachment.texture.pixelFormat;
    descriptor.stencilAttachmentPixelFormat = pass.stencilAttachment.texture.pixelFormat;
    descriptor.rasterSampleCount = pass.colorAttachments[0].texture.sampleCount;
    _pipeline = [device newRenderPipelineStateWithDescriptor:descriptor error:&error];
    MTLDepthStencilDescriptor *depth = [[MTLDepthStencilDescriptor alloc] init];
    depth.depthCompareFunction = MTLCompareFunctionLessEqual;
    depth.depthWriteEnabled = NO;
    _depthState = [device newDepthStencilStateWithDescriptor:depth];
    NSAssert(_pipeline && _depthState, @"probe setup failed: %@", error);
  }
  // Each frame counts into a buffer of its own, read once the GPU has finished the frame.
  id<MTLBuffer> counts = [device newBufferWithLength:2 * sizeof(uint32_t)
                                             options:MTLResourceStorageModeShared];
  memset(counts.contents, 0, 2 * sizeof(uint32_t));
  __weak MLNTerrainDepthProbeStyleLayer *weakSelf = self;
  [self.commandBuffer addCompletedHandler:^(id<MTLCommandBuffer>) {
    const uint32_t *counted = static_cast<const uint32_t *>(counts.contents);
    const uint32_t above = counted[0];
    const uint32_t below = counted[1];
    dispatch_async(dispatch_get_main_queue(), ^{
      weakSelf.aboveGroundFragments = above;
      weakSelf.belowGroundFragments = below;
      weakSelf.countedFrames += 1;
    });
  }];

  // A two-hundredth of the camera's height above the ground, and as far below it: the far
  // plane of a map seen from straight above is only a hundredth of that height under the
  // ground. At the default field of view the camera is one and a half view heights from the
  // center.
  const double cameraHeight = [mapView metersPerPointAtLatitude:context.centerCoordinate.latitude] *
                              1.5 * context.size.height;
  [encoder setRenderPipelineState:_pipeline];
  [encoder setDepthStencilState:_depthState];
  [encoder setCullMode:MTLCullModeNone];
  [encoder setFragmentBuffer:counts offset:0 atIndex:1];
  for (uint32_t which = 0; which < 2; ++which) {
    float depth = MLNDepthOverCenter(context, (which == 0 ? 1 : -1) * cameraHeight / 200.0);
    [encoder setVertexBytes:&depth length:sizeof(depth) atIndex:0];
    [encoder setFragmentBytes:&which length:sizeof(which) atIndex:0];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];
  }
  [self setNeedsDisplay];
}

@end
#endif

@interface MLNCustomStyleLayerTerrainTests : XCTestCase <MLNMapViewDelegate>

@property (nonatomic) MLNMapView *mapView;
@property (nonatomic) UIWindow *window;
@property (nonatomic) XCTestExpectation *styleLoadingExpectation;
@property (nonatomic) MLNTerrainRecordingStyleLayer *layer;

@end

@implementation MLNCustomStyleLayerTerrainTests

- (void)setUp {
  [super setUp];

  [MLNSettings setApiKey:@"pk.feedcafedeadbeefbadebede"];
  NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner"
                                                             withExtension:@"json"];
  CGRect bounds = CGRectMake(0, 0, 200, 200);
  self.mapView = [[MLNMapView alloc] initWithFrame:bounds styleURL:styleURL];
  self.mapView.delegate = self;

  // The custom layer is only drawn while the map view is on screen
  self.window = [[UIWindow alloc] initWithFrame:bounds];
  [self.window addSubview:self.mapView];
  [self.window makeKeyAndVisible];

  if (!self.mapView.style) {
    _styleLoadingExpectation =
        [self expectationWithDescription:@"Map view should finish loading style."];
    [self waitForExpectationsWithTimeout:10 handler:nil];
  }

  self.layer = [[MLNTerrainRecordingStyleLayer alloc] initWithIdentifier:@"terrain-recorder"];
  [self.mapView.style addLayer:self.layer];
}

- (void)tearDown {
  self.layer = nil;
  self.mapView = nil;
  self.window = nil;
  [MLNSettings setApiKey:nil];

  [super tearDown];
}

- (void)mapView:(MLNMapView *)mapView didFinishLoadingStyle:(MLNStyle *)style {
  [_styleLoadingExpectation fulfill];
}

- (void)waitForDrawWithTerrain:(BOOL)expectsTerrain {
  self.layer.expectsTerrain = expectsTerrain;
  self.layer.drawExpectation =
      [self expectationWithDescription:@"Custom layer should draw a frame."];
  [self.layer setNeedsDisplay];
  [self waitForExpectationsWithTimeout:10 handler:nil];
}

- (void)testTerrainOff {
  [self waitForDrawWithTerrain:NO];

  XCTAssertEqual(self.layer.drawnContext.terrainExaggeration, 0);
#if MLN_RENDER_BACKEND_METAL
  XCTAssertNil(self.layer.drawnDepthTexture);
#endif
}

- (void)testTerrainOn {
  // No DEM source backs the terrain, so the terrain is on but has no mesh or depth to
  // draw; this checks what the custom layer is handed, not the depth texture's contents.
  self.mapView.mbglMap.getStyle().setTerrain(std::make_unique<mln::style::Terrain>("dem", 1.5f));
  [self waitForDrawWithTerrain:YES];

  XCTAssertEqualWithAccuracy(self.layer.drawnContext.terrainExaggeration, 1.5, 1e-6);

  self.mapView.mbglMap.getStyle().setTerrain(nullptr);
  [self waitForDrawWithTerrain:NO];

  XCTAssertEqual(self.layer.drawnContext.terrainExaggeration, 0);
#if MLN_RENDER_BACKEND_METAL
  XCTAssertNil(self.layer.drawnDepthTexture);
#endif
}

- (void)testTerrainSetThroughTheStyleProperty {
  self.mapView.style.terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"dem"
                                                               exaggeration:1.5];
  [self waitForDrawWithTerrain:YES];

  XCTAssertEqualWithAccuracy(self.layer.drawnContext.terrainExaggeration, 1.5, 1e-6);

  self.mapView.style.terrain = nil;
  [self waitForDrawWithTerrain:NO];

  XCTAssertEqual(self.layer.drawnContext.terrainExaggeration, 0);
}

#if MLN_RENDER_BACKEND_METAL
// The attached depth buffer holds the mesh's depth, and a custom layer that remaps its clip z as
// the mesh does compares with it: what it draws just above the ground passes a less-or-equal
// depth test, and what it draws below the ground does not.
- (void)testDepthComparesWithTheTerrainMesh {
  [self checkDepthWithFirstFrameDelay:0 stall:20];
}

// The first frame drawn with terrain compiles the terrain's shaders, which can take far longer on
// a loaded machine than any fixed wait, and what a frame counted reaches the main queue only once
// the frame is over.
- (void)testDepthComparesWithTheTerrainMeshAfterASlowFirstFrame {
  [self checkDepthWithFirstFrameDelay:3 stall:2];
}

- (void)checkDepthWithFirstFrameDelay:(NSTimeInterval)delay stall:(NSTimeInterval)stall {
  // A flat elevation tile at sea level in the Mapbox encoding, the same for every tile.
  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  CGContextRef bitmap = CGBitmapContextCreate(NULL, 256, 256, 8, 0, colorSpace,
                                              static_cast<uint32_t>(kCGImageAlphaNoneSkipLast) |
                                                  static_cast<uint32_t>(kCGBitmapByteOrder32Big));
  CGContextSetRGBFillColor(bitmap, 1.0 / 255.0, 134.0 / 255.0, 160.0 / 255.0, 1.0);
  CGContextFillRect(bitmap, CGRectMake(0, 0, 256, 256));
  CGImageRef image = CGBitmapContextCreateImage(bitmap);
  NSData *png = UIImagePNGRepresentation([UIImage imageWithCGImage:image]);
  CGImageRelease(image);
  CGContextRelease(bitmap);
  CGColorSpaceRelease(colorSpace);
  NSURL *tileURL = [[NSURL fileURLWithPath:NSTemporaryDirectory()]
      URLByAppendingPathComponent:@"MLNCustomStyleLayerTerrainTests-flat-dem.png"];
  XCTAssertTrue([png writeToURL:tileURL atomically:YES]);

  MLNRasterDEMSource *dem =
      [[MLNRasterDEMSource alloc] initWithIdentifier:@"dem"
                                    tileURLTemplates:@[ tileURL.absoluteString ]
                                             options:@{
                                               MLNTileSourceOptionTileSize : @256,
                                               MLNTileSourceOptionMaximumZoomLevel : @0
                                             }];
  [self.mapView.style addSource:dem];
  [self.mapView setCenterCoordinate:CLLocationCoordinate2DMake(0, 0) zoomLevel:10 animated:NO];
  MLNTerrainDepthProbeStyleLayer *probe =
      [[MLNTerrainDepthProbeStyleLayer alloc] initWithIdentifier:@"terrain-depth-probe"];
  probe.firstFrameDelay = delay;
  [self.mapView.style addLayer:probe];
  self.mapView.mbglMap.getStyle().setTerrain(std::make_unique<mln::style::Terrain>("dem", 1.0f));

  // The mesh is drawn once its elevation tile is in: until then nothing hides the quad below
  // the ground either. A frame can take any time to render, so the wait ends when the layer has
  // gone `stall` seconds without a frame drawn or counted, not a fixed time after it began; a map
  // that keeps drawing without the mesh ever hiding the quad ends it after a thousand frames.
  NSUInteger progress = 0;
  NSDate *lastProgress = [NSDate date];
  while (!(probe.countedFrames > 0 && probe.belowGroundFragments == 0 &&
           probe.aboveGroundFragments > 0) &&
         [lastProgress timeIntervalSinceNow] > -stall && probe.drawnFrames < 1000) {
    [probe setNeedsDisplay];
    [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];
    if (probe.drawnFrames + probe.countedFrames != progress) {
      progress = probe.drawnFrames + probe.countedFrames;
      lastProgress = [NSDate date];
    }
  }
  XCTAssertGreaterThan(probe.countedFrames, 0u);
  XCTAssertEqual(probe.belowGroundFragments, 0u, @"the mesh hides what is under the ground");
  XCTAssertGreaterThan(probe.aboveGroundFragments, 0u, @"and nothing that is over it");

  self.mapView.mbglMap.getStyle().setTerrain(nullptr);
  [self.mapView.style removeLayer:probe];
}
#endif

@end
