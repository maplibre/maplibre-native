#import <MapLibre.h>
#import <XCTest/XCTest.h>

#import "MBXNationalGridProjection.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace {

// The eastings and northings of the crosshairs the `projection/national-grid/crosshairs` render
// test's tiles carry.
constexpr std::array<std::array<double, 2>, 4> MLNTestCrosshairs = {{
    {310425, 386624},
    {452889, 279104},
    {611481, 94528},
    {274585, 193088},
}};

// Half the side of the square measured around a crosshair, whose arms are 20 points long, and how
// far from its center an arm counts, so each arm measures one axis. As the Android test measures.
constexpr double MLNTestWindowPoints = 30;
constexpr double MLNTestArmGapPoints = 4;

struct MLNTestCentroid {
  double sumX = 0;
  double sumY = 0;
  double weight = 0;
  void add(double x, double y, double w) {
    sumX += x * w;
    sumY += y * w;
    weight += w;
  }
  double x() const { return sumX / weight; }
  double y() const { return sumY / weight; }
};

}  // namespace

@interface MLNNationalGridTests : XCTestCase
@end

@implementation MLNNationalGridTests

- (void)setUp {
  [super setUp];
  NSError *error;
  XCTAssertTrue([MLNStyle addProjection:MBXNationalGridProjection() error:&error], @"%@", error);
}

- (void)tearDown {
  XCTAssertTrue([MLNStyle removeProjectionNamed:@"EPSG:27700" error:nil]);
  [super tearDown];
}

// W11's predicate: each GeoJSON ring, placed at the WGS84 position the projection's inverse gives
// for a crosshair, lands within 1 logical pixel of that crosshair in the tiles.
- (void)testRingsLandOnTheirCrosshairs {
  NSURL *styleURL = MBXNationalGridCrosshairsStyleURL([NSBundle bundleForClass:[self class]]);
  XCTAssertNotNil(styleURL);

  MLNMapCamera *camera = [MLNMapCamera camera];
  camera.centerCoordinate = CLLocationCoordinate2DMake(51.96064818208741, -1.2773320477729189);
  MLNMapSnapshotOptions *options =
      [[MLNMapSnapshotOptions alloc] initWithStyleURL:styleURL
                                               camera:camera
                                                 size:CGSizeMake(512, 512)];
  options.zoomLevel = 2;
  options.showsLogo = NO;
  options.showsAttribution = NO;

  MLNMapSnapshotter *snapshotter = [[MLNMapSnapshotter alloc] initWithOptions:options];
  XCTestExpectation *expectation = [self expectationWithDescription:@"The snapshot is taken"];
  __block MLNMapSnapshot *snapshot;
  [snapshotter startWithCompletionHandler:^(MLNMapSnapshot *result, NSError *error) {
    XCTAssertNil(error);
    snapshot = result;
    [expectation fulfill];
  }];
  [self waitForExpectations:@[ expectation ] timeout:30];
  XCTAssertNotNil(snapshot);

  CGImageRef image = snapshot.image.CGImage;
  const size_t width = CGImageGetWidth(image);
  const size_t height = CGImageGetHeight(image);
  const double scale = snapshot.image.scale;
  std::vector<uint8_t> pixels(width * height * 4);
  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  CGContextRef context = CGBitmapContextCreate(pixels.data(), width, height, 8, width * 4,
                                               colorSpace, kCGImageAlphaPremultipliedLast);
  CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
  CGContextRelease(context);
  CGColorSpaceRelease(colorSpace);

  MLNCoordinateReferenceSystem *projection = MBXNationalGridProjection();
  const double window = MLNTestWindowPoints * scale;
  const double armGap = MLNTestArmGapPoints * scale;
  for (const auto &[easting, northing] : MLNTestCrosshairs) {
    CLLocationCoordinate2D coordinate = projection.inverse({easting, northing});
    CGPoint expected = [snapshot pointForCoordinate:coordinate];
    const double ex = expected.x * scale;
    const double ey = expected.y * scale;

    MLNTestCentroid vertical, horizontal, ring;
    const auto top = static_cast<size_t>(std::max(0.0, ey - window));
    const auto bottom = static_cast<size_t>(std::min<double>(height - 1, ey + window));
    const auto left = static_cast<size_t>(std::max(0.0, ex - window));
    const auto right = static_cast<size_t>(std::min<double>(width - 1, ex + window));
    for (size_t y = top; y <= bottom; y++) {
      for (size_t x = left; x <= right; x++) {
        const uint8_t *pixel = &pixels[(y * width + x) * 4];
        const int r = pixel[0];
        const int g = pixel[1];
        const int b = pixel[2];
        const double red = std::max(0, r - std::max(g, b));
        const double blue = std::max(0, b - std::max(r, g));
        const double cx = x + 0.5;
        const double cy = y + 0.5;
        if (std::abs(cy - ey) > armGap) {
          vertical.add(cx, cy, red);
        }
        if (std::abs(cx - ex) > armGap) {
          horizontal.add(cx, cy, red);
        }
        ring.add(cx, cy, blue);
      }
    }
    XCTAssertGreaterThan(vertical.weight, 0, @"The crosshair at %.0f E %.0f N is drawn", easting,
                         northing);
    XCTAssertGreaterThan(horizontal.weight, 0, @"The crosshair at %.0f E %.0f N is drawn", easting,
                         northing);
    XCTAssertGreaterThan(ring.weight, 0, @"The ring at %.0f E %.0f N is drawn", easting, northing);

    const double offset = std::hypot(ring.x() - vertical.x(), ring.y() - horizontal.y()) / scale;
    NSLog(@"National Grid crosshair %.0f E %.0f N at (%.3f, %.3f), ring at (%.3f, %.3f): %.3f "
          @"logical px "
          @"apart",
          easting, northing, vertical.x() / scale, horizontal.y() / scale, ring.x() / scale,
          ring.y() / scale, offset);
    XCTAssertLessThanOrEqual(offset, 1, @"The ring at %.0f E %.0f N", easting, northing);
  }
}

@end
