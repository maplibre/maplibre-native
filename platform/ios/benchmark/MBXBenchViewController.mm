#import "MBXBenchViewController.h"
#import "MBXBenchAppDelegate.h"
#import "MLNMapView_Private.h"
#import "MLNOfflineStorage_Private.h"
#import "MLNSettings_Private.h"

#include "BenchmarkAdvancedMetrics.h"
#include "locations.hpp"

#include <sys/utsname.h>
#include <algorithm>
#include <chrono>
#include <numeric>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/map.hpp>
#include <mln/style/style.hpp>

@protocol BenchMapDelegate <NSObject>
- (void)mapDidFinishRenderingFrameFullyRendered:(BOOL)fullyRendered
                              frameEncodingTime:(double)frameEncodingTime
                             frameRenderingTime:(double)frameRenderingTime;
@end

class BenchMapObserver : public mln::MapObserver {
public:
  BenchMapObserver() = delete;
  BenchMapObserver(id<BenchMapDelegate> mapDelegate_) : mapDelegate(mapDelegate_) {}
  virtual ~BenchMapObserver() = default;

  void onDidFinishRenderingFrame(const RenderFrameStatus &status) override final {
    // NSLog(@"Frame encoding time: %4.1f ms", status.renderingStats.encodingTime * 1e3);
    // NSLog(@"Frame rendering time: %4.1f ms", status.renderingStats.renderingTime * 1e3);

    bool fullyRendered = status.mode == mln::MapObserver::RenderMode::Full;
    [mapDelegate mapDidFinishRenderingFrameFullyRendered:fullyRendered
                                       frameEncodingTime:status.renderingStats->encodingTime
                                      frameRenderingTime:status.renderingStats->renderingTime];
  }

protected:
  __weak id<BenchMapDelegate> mapDelegate = nullptr;
};

@interface MBXBenchViewController () <BenchMapDelegate> {
  std::unique_ptr<BenchMapObserver> observer;
  std::unique_ptr<mln::HeadlessFrontend> frontend;
  std::unique_ptr<mln::Map> map;
}

@property (nonatomic) UIImageView *imageView;

@end

@implementation MBXBenchViewController

// MARK: - Setup

+ (void)initialize {
  if (self == [MBXBenchViewController class]) {
    [[NSUserDefaults standardUserDefaults] registerDefaults:@{
      @"MBXUserTrackingMode" : @(MLNUserTrackingModeNone),
      @"MBXShowsUserLocation" : @NO,
      @"MBXDebug" : @NO,
    }];
  }
}

- (void)viewDidLoad {
  [super viewDidLoad];
}

- (void)viewDidAppear:(BOOL)animated {
  [super viewDidAppear:animated];

#ifdef LOG_TO_DOCUMENTS_DIR
  [self setAndRedirectLogFileToDocuments];
#endif

  // Use a local style and local assets if they’ve been downloaded.
  NSURL *tile = [[NSBundle mainBundle] URLForResource:@"11"
                                        withExtension:@"pbf"
                                         subdirectory:@"tiles/tiles/v3/5/7"];
  NSURL *tileSourceURL = [[NSBundle mainBundle] URLForResource:@"openmaptiles"
                                                 withExtension:@"json"
                                                  subdirectory:@"tiles"];
  // Keyless, so the benchmark runs without an API key (e.g. on CI).
  const std::vector<std::pair<NSString *, NSString *>> styles = {
      {@"OpenFreeMap Bright", @"https://tiles.openfreemap.org/styles/bright"},
  };
  constexpr auto styleIndex = 0;
  styleName = tile ? @"MapTiler Streets (bundled assets)" : styles[styleIndex].first;
  NSURL *url =
      [NSURL URLWithString:tile ? @"asset://styles/streets.json" : styles[styleIndex].second];
  NSLog(@"Using style URL: \"%@\"", [url absoluteString]);

  self.imageView = [[UIImageView alloc] initWithFrame:self.view.bounds];
  self.imageView.autoresizingMask =
      UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
  [self.view addSubview:self.imageView];

  mln::Size viewSize = {static_cast<uint32_t>(self.view.bounds.size.width),
                        static_cast<uint32_t>(self.view.bounds.size.height)};
  auto pixelRatio = [[UIScreen mainScreen] scale];

  observer = std::make_unique<BenchMapObserver>(self);
  frontend = std::make_unique<mln::HeadlessFrontend>(
      viewSize, pixelRatio, mln::gfx::HeadlessBackend::SwapBehaviour::Flush,
      mln::gfx::ContextMode::Unique,
      /* localFontFamily */ std::nullopt,
      /* invalidateOnUpdate */ false);

  mln::MapOptions mapOptions;
  mapOptions.withMapMode(mln::MapMode::Continuous)
      .withSize(viewSize)
      .withPixelRatio(pixelRatio)
      .withConstrainMode(mln::ConstrainMode::None)
      .withViewportMode(mln::ViewportMode::Default)
      .withCrossSourceCollisions(true);

  mln::TileServerOptions *tileServerOptions =
      [[MLNSettings sharedSettings] tileServerOptionsInternal];
  mln::ResourceOptions resourceOptions;
  resourceOptions.withCachePath(MLNOfflineStorage.sharedOfflineStorage.databasePath.UTF8String)
      .withAssetPath([NSBundle mainBundle].resourceURL.path.UTF8String)
      .withTileServerOptions(*tileServerOptions);
  mln::ClientOptions clientOptions;

  auto apiKey = [[MLNSettings sharedSettings] apiKey];
  if (apiKey) {
    resourceOptions.withApiKey([apiKey UTF8String]);
  }

  map =
      std::make_unique<mln::Map>(*frontend, *observer, mapOptions, resourceOptions, clientOptions);
  map->setSize(viewSize);
  map->setDebug(mln::MapDebugOptions::NoDebug);
  map->getStyle().loadURL([url.absoluteString UTF8String]);

  [self startBenchmarkIteration];
}

// For setting the filename
NSDate *const currentDate = [NSDate date];

/*!
 \description
  Write the Benchmark log to the Documents Directory.  For the device or simulator, fetch the
 Documents Directory, and make a friendly name for the benchmarking output file.  In Build Settings
 > Other C Flags add the compiler flag. `-DLOG_TO_DOCUMENTS_DIR`
 */
- (void)setAndRedirectLogFileToDocuments {
  NSString *const name = UIDevice.currentDevice.name;
#if TARGET_IPHONE_SIMULATOR
  NSString *const DeviceMode = @"Simulator";
#else
  NSString *const DeviceMode = @"Device";
#endif

  // Set log file name date
  NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
  [formatter setDateFormat:@"yyyy-MM-dd-HHmm"];
  NSString *dateString = [formatter stringFromDate:currentDate];

  // Set the log file name
  NSString *filename =
      [NSString stringWithFormat:@"MapLibre-bench-%@-%@-%@.log", name, DeviceMode, dateString];

  // Set log file path
  NSArray *allPaths =
      NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
  NSString *documentsDirectory = [allPaths objectAtIndex:0];
  NSString *pathForLog = [documentsDirectory stringByAppendingPathComponent:filename];
  NSLog(@"Writing MapLibre Bench log.  To open in Console, use the CLI command");
  NSLog(@"  open \"%@\"", pathForLog);

  [self redirectLogToDocuments:pathForLog];
}

- (void)redirectLogToDocuments:(NSString *)fileName {
  // write to file in append "a+" mode
  freopen([fileName cStringUsingEncoding:NSASCIIStringEncoding], "a+", stderr);
}

size_t idx = 0;
enum class State { None, WaitingForAssets, Benchmarking } state = State::None;
int frames = 0;
double totalFrameEncodingTime = 0;
double totalFrameRenderingTime = 0;
BenchmarkAdvancedMetrics *advancedMetrics = nil;
std::chrono::steady_clock::time_point started;
std::vector<std::pair<std::string, std::pair<double, double>>> result;
NSString *styleName = nil;
// Over all locations: the encoding and rendering time of every frame in milliseconds, and the time
// spent benchmarking.
std::vector<double> frameEncodingTimes;
std::vector<double> frameRenderingTimes;
double benchmarkingSeconds = 0;

static const int benchmarkDuration = 5;  // seconds

namespace mln {
extern std::size_t uploadCount, uploadBuildCount, uploadVertextAttrsDirty, uploadInvalidSegments;
}

- (void)renderFrame {
  mln::gfx::BackendScope guard{*(frontend->getBackend())};

  frontend->renderFrame();

  auto image = frontend->readStillImage();

  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  CGContextRef bitmapContext =
      CGBitmapContextCreate(image.data.get(), image.size.width, image.size.height, 8,
                            4 * image.size.width, colorSpace, kCGImageAlphaPremultipliedLast);
  CFRelease(colorSpace);
  CGImageRef cgImage = CGBitmapContextCreateImage(bitmapContext);
  CGContextRelease(bitmapContext);

  self.imageView.image = [UIImage imageWithCGImage:cgImage];
  CGImageRelease(cgImage);
}

- (void)startBenchmarkIteration {
  if (mln::bench::locations.size() > idx) {
    if (advancedMetrics == nil) {
      advancedMetrics = [[BenchmarkAdvancedMetrics alloc] init];
      [advancedMetrics start:1];
    }

    const auto &location = mln::bench::locations[idx];

    mln::CameraOptions cameraOptions;
    cameraOptions.center = mln::LatLng(location.latitude, location.longitude);
    cameraOptions.zoom = location.zoom;
    cameraOptions.bearing = location.bearing;
    mln::AnimationOptions animationOptions;
    animationOptions.duration.emplace(std::chrono::duration_cast<mln::Duration>(
        std::chrono::duration<NSTimeInterval>(benchmarkDuration)));
    map->easeTo(cameraOptions, animationOptions);

    state = State::WaitingForAssets;
    NSLog(@"Benchmarking \"%s\"", location.name.c_str());
    NSLog(@"- Loading assets...");

    dispatch_async(dispatch_get_main_queue(), ^{
      [self renderFrame];
    });
  } else {
    // Do nothing. The benchmark is completed.
    NSLog(@"Benchmark completed.");

    NSLog(@"Result:");
    size_t colWidth = 0;
    for (const auto &row : result) {
      colWidth = std::max(row.first.size(), colWidth);
    }

    double totalFrameEncodingTime = 0;
    double totalFrameRenderingTime = 0;
    for (const auto &row : result) {
      NSLog(@"| %-*s | %4.2f ms | %4.2f ms |", int(colWidth), row.first.c_str(),
            1e3 * row.second.first, 1e3 * row.second.second);
      totalFrameEncodingTime += row.second.first;
      totalFrameRenderingTime += row.second.second;
    }

    NSLog(@"Average frame encoding time: %4.2f ms", totalFrameEncodingTime * 1e3 / result.size());
    NSLog(@"Average frame rendering time: %4.2f ms", totalFrameRenderingTime * 1e3 / result.size());

    // NSLog(@"Total uploads: %zu", mln::uploadCount);
    // NSLog(@"Total uploads with dirty vattr: %zu", mln::uploadVertextAttrsDirty);
    // NSLog(@"Total uploads with invalid segs: %zu", mln::uploadInvalidSegments);
    // NSLog(@"Total uploads with build: %zu", mln::uploadBuildCount);

    [advancedMetrics stop];

    NSLog(@"Cpu usage: (min %.2f%%) (max %.2f%%) (avg %.2f%%)", advancedMetrics.min.cpu,
          advancedMetrics.max.cpu, advancedMetrics.avg.cpu);
    NSLog(@"Memory usage: (min %llu MB) (max %llu MB) (avg %llu MB)",
          advancedMetrics.min.memory / 0x100000, advancedMetrics.max.memory / 0x100000,
          advancedMetrics.avg.memory / 0x100000);

    NSData *results = [self writeResults];

    advancedMetrics = nil;

#if !defined(NDEBUG)
    // Clean up and show rendering stats, as in `destroyCoreObjects` from tests.
    // TODO: This doesn't clean up everything, what are we missing?
    map.reset();
    observer.reset();
    frontend.reset();
#endif  // !defined(NDEBUG)

    // BenchmarkTests, which runs inside this app, waits for this notification.
    [[NSNotificationCenter defaultCenter] postNotificationName:@"MBXBenchmarkDidFinish"
                                                        object:self
                                                      userInfo:@{@"results" : results}];
  }
}

static double average(const std::vector<double> &values) {
  return values.empty() ? 0 : std::accumulate(values.begin(), values.end(), 0.0) / values.size();
}

// Average of the slowest 1% of the values, like FrameTimeStore.low1p() in the Android benchmark.
static double low1p(std::vector<double> values) {
  std::sort(values.begin(), values.end());
  return average(std::vector<double>(values.begin() + 99 * values.size() / 100, values.end()));
}

// Writes Documents/benchmark_results.json in the format of the Android benchmark's
// benchmark_results.json (see docs/mdbook/src/platforms/android/benchmark.md) and returns its
// contents. The git revision is added to Info.plist by build-benchmark-for-device-farm.sh.
- (NSData *)writeResults {
  auto memoryInMB = [](unsigned long long bytes) { return @(bytes / 0x100000); };
  NSDictionary *result = @{
    @"styleName" : styleName,
    @"syncRendering" : @YES,
    @"thermalState" : @(NSProcessInfo.processInfo.thermalState),
    @"fps" : @(benchmarkingSeconds > 0 ? frameEncodingTimes.size() / benchmarkingSeconds : 0),
    @"avgEncodingTime" : @(average(frameEncodingTimes)),
    @"avgRenderingTime" : @(average(frameRenderingTimes)),
    @"low1pEncodingTime" : @(low1p(frameEncodingTimes)),
    @"low1pRenderingTime" : @(low1p(frameRenderingTimes)),
    @"cpu" : @{
      @"min" : @(advancedMetrics.min.cpu),
      @"max" : @(advancedMetrics.max.cpu),
      @"avg" : @(advancedMetrics.avg.cpu),
    },
    @"memory" : @{
      @"min" : memoryInMB(advancedMetrics.min.memory),
      @"max" : memoryInMB(advancedMetrics.max.memory),
      @"avg" : memoryInMB(advancedMetrics.avg.memory),
    },
  };

  struct utsname systemInfo;
  uname(&systemInfo);
#ifdef NDEBUG
  const BOOL debugBuild = NO;
#else
  const BOOL debugBuild = YES;
#endif
  NSDictionary *payload = @{
    @"results" : @[ result ],
    @"deviceManufacturer" : @"Apple",
    @"model" : @(systemInfo.machine),
    @"renderer" : @"metal",
    @"debugBuild" : @(debugBuild),
    @"gitRevision" : [NSBundle.mainBundle objectForInfoDictionaryKey:@"MLNGitRevision"] ?: @"",
    @"timestamp" : @((long long)(NSDate.date.timeIntervalSince1970 * 1000)),
  };

  NSData *json = [NSJSONSerialization dataWithJSONObject:payload options:0 error:nil];
  NSURL *documents = [NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory
                                                          inDomains:NSUserDomainMask]
                         .firstObject;
  NSURL *file = [documents URLByAppendingPathComponent:@"benchmark_results.json"];
  [json writeToURL:file atomically:YES];
  NSLog(@"Wrote %@", file.path);
  return json;
}

- (void)mapDidFinishRenderingFrameFullyRendered:(__unused BOOL)fullyRendered
                              frameEncodingTime:(double)frameEncodingTime
                             frameRenderingTime:(double)frameRenderingTime {
  if (state == State::Benchmarking) {
    frames++;
    totalFrameEncodingTime += frameEncodingTime;
    totalFrameRenderingTime += frameRenderingTime;
    frameEncodingTimes.push_back(frameEncodingTime * 1e3);
    frameRenderingTimes.push_back(frameRenderingTime * 1e3);

    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                              std::chrono::steady_clock::now() - started)
                              .count();
    if (duration >= benchmarkDuration * 1e6) {
      state = State::None;
      benchmarkingSeconds += duration / 1e6;

      // Report FPS
      const auto frameEncodingTime = static_cast<double>(totalFrameEncodingTime) / frames;
      const auto frameRenderingTime = static_cast<double>(totalFrameRenderingTime) / frames;
      result.emplace_back(mln::bench::locations[idx].name,
                          std::make_pair(frameEncodingTime, frameRenderingTime));
      NSLog(@"- Frame encoding time: %.1f ms, Frame rendering time: %.1f ms (%d frames)",
            frameEncodingTime * 1e3, frameRenderingTime * 1e3, frames);

      // Start benchmarking the next location.
      idx++;
      [self startBenchmarkIteration];
    } else {
      dispatch_async(dispatch_get_main_queue(), ^{
        [self renderFrame];
      });
    }
    return;
  } else if (state == State::WaitingForAssets) {
    if (map->isFullyLoaded()) {
      // Start the benchmarking timer.
      frames = 0;
      totalFrameEncodingTime = 0;
      totalFrameRenderingTime = 0;
      state = State::Benchmarking;
      started = std::chrono::steady_clock::now();
      NSLog(@"- Benchmarking for %d seconds...", benchmarkDuration);
    }
    dispatch_async(dispatch_get_main_queue(), ^{
      [self renderFrame];
    });
    return;
  }
}

- (NSUInteger)supportedInterfaceOrientations {
  return UIInterfaceOrientationMaskLandscape;
}

@end
