#import "MBXBenchViewController.h"
#import "MBXBenchAppDelegate.h"
#import "MLNMapView_Private.h"
#import "MLNOfflineStorage_Private.h"
#import "MLNSettings_Private.h"

#include "file_source.hpp"
#include "locations.hpp"

#include <chrono>

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/map/map.hpp>
#include <mbgl/storage/file_source_manager.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/util/run_loop.hpp>

@protocol BenchMapDelegate <NSObject>
- (void)mapDidFinishRenderingFrameFullyRendered:(BOOL)fullyRendered
                              frameEncodingTime:(double)frameEncodingTime
                             frameRenderingTime:(double)frameRenderingTime;
@end


class BenchMapObserver : public mbgl::MapObserver {
public:
    BenchMapObserver() = delete;
    BenchMapObserver(id<BenchMapDelegate> mapDelegate_) : mapDelegate(mapDelegate_) {}
    virtual ~BenchMapObserver() = default;
    
    void onDidFinishRenderingFrame(RenderFrameStatus status) override final {
        //NSLog(@"Frame encoding time: %4.1f ms", status.frameEncodingTime * 1e3);
        //NSLog(@"Frame rendering time: %4.1f ms", status.frameRenderingTime * 1e3);
        
        bool fullyRendered = status.mode == mbgl::MapObserver::RenderMode::Full;
        [mapDelegate mapDidFinishRenderingFrameFullyRendered:fullyRendered 
                                           frameEncodingTime:status.frameEncodingTime
                                          frameRenderingTime:status.frameRenderingTime];
    }
    
protected:
    __weak id<BenchMapDelegate> mapDelegate = nullptr;
};

@interface MBXBenchViewController () <BenchMapDelegate> {
    std::unique_ptr<BenchMapObserver> observer;
    std::unique_ptr<mbgl::HeadlessFrontend> frontend;
    std::unique_ptr<mbgl::Map> map;
    mbgl::util::RunLoop runLoop;
}

@property (nonatomic) UIImageView *imageView;

@end

@implementation MBXBenchViewController

// MARK: - Setup

+ (void)initialize
{
    if (self == [MBXBenchViewController class])
    {
        [[NSUserDefaults standardUserDefaults] registerDefaults:@{
            @"MBXUserTrackingMode": @(MLNUserTrackingModeNone),
            @"MBXShowsUserLocation": @NO,
            @"MBXDebug": @NO,
        }];
    }
}

- (void)viewDidLoad
{
    [super viewDidLoad];
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    
    #ifdef LOG_TO_DOCUMENTS_DIR
    [self setAndRedirectLogFileToDocuments];
    #endif
    
    // Use a local style and local assets if they’ve been downloaded.
    NSURL *tile = [[NSBundle mainBundle] URLForResource:@"11" withExtension:@"pbf" subdirectory:@"tiles/tiles/v3/5/7"];
    NSURL *tileSourceURL = [[NSBundle mainBundle] URLForResource:@"openmaptiles" withExtension:@"json" subdirectory:@"tiles"];
    const std::vector<std::string> styles = {
        "maptiler://maps/streets",
    };
    constexpr auto styleIndex = 0;
    NSURL *url = [NSURL URLWithString:tile ? @"asset://styles/streets.json" : [NSString stringWithCString:styles[styleIndex].c_str() encoding:NSUTF8StringEncoding]];
    NSLog(@"Using style URL: \"%@\"", [url absoluteString]);

    self.imageView = [[UIImageView alloc] initWithFrame:self.view.bounds];
    self.imageView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self.view addSubview:self.imageView];
    
    //mbgl::Size viewSize = { static_cast<uint32_t>(self.view.bounds.size.width),
    //                        static_cast<uint32_t>(self.view.bounds.size.height) };
    mbgl::Size viewSize = { 512, 512 };
    auto pixelRatio = [[UIScreen mainScreen] scale];
    
    observer = std::make_unique<BenchMapObserver>(self);
    frontend = std::make_unique<mbgl::HeadlessFrontend>(
        viewSize,
        pixelRatio,
        mbgl::gfx::HeadlessBackend::SwapBehaviour::Flush,
        mbgl::gfx::ContextMode::Unique,
        /* localFontFamily */ std::nullopt,
        /* invalidateOnUpdate */ true
    );
    
    mbgl::MapOptions mapOptions;
    mapOptions.withMapMode(mbgl::MapMode::Static)
              .withSize(viewSize)
              .withPixelRatio(pixelRatio)
              .withConstrainMode(mbgl::ConstrainMode::None)
              .withViewportMode(mbgl::ViewportMode::Default)
              .withCrossSourceCollisions(true);
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSString* cachePath = [documentsDirectory stringByAppendingPathComponent:@"cache-style.db"];
    
    mbgl::TileServerOptions* tileServerOptions = [[MLNSettings sharedSettings] tileServerOptionsInternal];
    mbgl::ResourceOptions resourceOptions;
    resourceOptions.withCachePath(cachePath.UTF8String)
                   .withAssetPath([NSBundle mainBundle].resourceURL.path.UTF8String)
                   .withTileServerOptions(*tileServerOptions);
    mbgl::ClientOptions clientOptions;
    
    auto* fileSourceManager = mbgl::FileSourceManager::get();

    auto resourceLoaderFactory = fileSourceManager->unRegisterFileSourceFactory(
        mbgl::FileSourceType::ResourceLoader);
    auto factory = [defaultFactory = std::move(resourceLoaderFactory)](const mbgl::ResourceOptions& resourceOptions,
                                                                       const mbgl::ClientOptions& clientOptions) {
        assert(defaultFactory);
        std::shared_ptr<mbgl::FileSource> fileSource = defaultFactory(resourceOptions, clientOptions);
        return std::make_unique<mbgl::ProxyFileSource>(std::move(fileSource), resourceOptions, clientOptions);
    };

    fileSourceManager->registerFileSourceFactory(mbgl::FileSourceType::ResourceLoader, std::move(factory));
    
    auto apiKey = [[MLNSettings sharedSettings] apiKey];
    if (apiKey) {
        resourceOptions.withApiKey([apiKey UTF8String]);
    }
    
    map = std::make_unique<mbgl::Map>(*frontend, *observer, mapOptions, resourceOptions, clientOptions);
    map->setSize(viewSize);
    map->setDebug(mbgl::MapDebugOptions::NoDebug);
    //map->getStyle().loadURL([url.absoluteString UTF8String]);
    
    NSString* jsonPath = [documentsDirectory stringByAppendingPathComponent:@"style.json"];
    NSData* jsonData = [NSData dataWithContentsOfFile:jsonPath];
    NSString* jsonString = [[NSString alloc] initWithBytes:[jsonData bytes]
                                                    length:[jsonData length]
                                                  encoding:NSUTF8StringEncoding];
    
    map->getStyle().loadJSON(jsonString.UTF8String);
    //map->setDebug(mbgl::MapDebugOptions::Collision | mbgl::MapDebugOptions::TileBorders);
    
    //map->getStyle().loadURL("https://api.maptiler.com/maps/2f4c4899-e695-4c8b-a4ef-92fa9d2740ff/style.json?key=G4MQXsYbLiUxOu3SV4lh");
    
    //[self startBenchmarkIteration];
    [self renderFrame];
}

// For setting the filename
NSDate* const currentDate = [NSDate date];

/*!
 \description
  Write the Benchmark log to the Documents Directory.  For the device or simulator, fetch the Documents Directory, and make a friendly name for the benchmarking output file.  In Build Settings > Other C Flags add the compiler flag. `-DLOG_TO_DOCUMENTS_DIR`
 */
- (void)setAndRedirectLogFileToDocuments
{
    NSString* const name = UIDevice.currentDevice.name;
    #if TARGET_IPHONE_SIMULATOR
        NSString* const DeviceMode = @"Simulator";
    #else
        NSString* const DeviceMode = @"Device";
    #endif

    // Set log file name date
    NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
    [formatter setDateFormat:@"yyyy-MM-dd-HHmm"];
    NSString *dateString = [formatter stringFromDate:currentDate];
    
    // Set the log file name
    NSString* filename = [NSString stringWithFormat: @"MapLibre-bench-%@-%@-%@.log", name, DeviceMode, dateString];

    // Set log file path
    NSArray *allPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [allPaths objectAtIndex:0];
    NSString *pathForLog = [documentsDirectory stringByAppendingPathComponent: filename];
    NSLog(@"Writing MapLibre Bench log.  To open in Console, use the CLI command");
    NSLog(@"  open \"%@\"", pathForLog);

    [self redirectLogToDocuments :pathForLog];
}

- (void)redirectLogToDocuments: (NSString*) fileName
{
    // write to file in append "a+" mode
    freopen([fileName cStringUsingEncoding:NSASCIIStringEncoding], "a+", stderr);
}

size_t idx = 0;
enum class State { None, WaitingForAssets, WarmingUp, Benchmarking } state = State::None;
int frames = 0;
double totalFrameEncodingTime = 0;
double totalFrameRenderingTime = 0;
std::chrono::steady_clock::time_point started;
std::vector<std::pair<std::string, std::pair<double, double>> > result;

static const int warmupDuration = 20; // frames
static const int benchmarkDuration = 200; // frames

namespace  mbgl {
    extern std::size_t uploadCount, uploadBuildCount, uploadVertextAttrsDirty, uploadInvalidSegments;
}

- (void)renderFrame
{
    mbgl::gfx::BackendScope guard{*(frontend->getBackend())};
    
    //frontend->renderFrame();
    
    //auto image = frontend->readStillImage();
    
    auto result = frontend->render(*map);
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef bitmapContext = CGBitmapContextCreate(
       result.image.data.get(),
       result.image.size.width,
       result.image.size.height,
       8,
       4 * result.image.size.width,
       colorSpace,
       kCGImageAlphaPremultipliedLast | kCGBitmapByteOrderDefault
    );
    CFRelease(colorSpace);
    CGImageRef cgImage = CGBitmapContextCreateImage(bitmapContext);
    CGContextRelease(bitmapContext);
    
    self.imageView.image = [UIImage imageWithCGImage:cgImage];
    CGImageRelease(cgImage);
}

- (void)startBenchmarkIteration
{
    if (mbgl::bench::locations.size() > idx) {
        const auto& location = mbgl::bench::locations[idx];
        
        mbgl::CameraOptions cameraOptions;
        cameraOptions.center = mbgl::LatLng(location.latitude, location.longitude);
        cameraOptions.zoom = location.zoom;
        cameraOptions.bearing = location.bearing;
        map->jumpTo(cameraOptions);
        
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
        for (const auto& row : result) {
            colWidth = std::max(row.first.size(), colWidth);
        }
        
        double totalFrameEncodingTime = 0;
        double totalFrameRenderingTime = 0;
        for (const auto& row : result) {
            NSLog(@"| %-*s | %4.1f ms | %4.1f ms |", int(colWidth), row.first.c_str(), 1e3 * row.second.first, 1e3 * row.second.second);
            totalFrameEncodingTime += row.second.first;
            totalFrameRenderingTime += row.second.second;
        }

        NSLog(@"Average frame encoding time: %4.1f ms", totalFrameEncodingTime * 1e3 / result.size());
        NSLog(@"Average frame rendering time: %4.1f ms", totalFrameRenderingTime * 1e3 / result.size());
        
        // NSLog(@"Total uploads: %zu", mbgl::uploadCount);
        // NSLog(@"Total uploads with dirty vattr: %zu", mbgl::uploadVertextAttrsDirty);
        // NSLog(@"Total uploads with invalid segs: %zu", mbgl::uploadInvalidSegments);
        // NSLog(@"Total uploads with build: %zu", mbgl::uploadBuildCount);

#if !defined(NDEBUG)
        // Clean up and show rendering stats, as in `destroyCoreObjects` from tests.
        // TODO: This doesn't clean up everything, what are we missing?
        map.reset();
        observer.reset();
        frontend.reset();
#endif // !defined(NDEBUG)

        // this does not shut the application down correctly,
        // and results in an assertion failure in thread-local code
        //exit(0);

        // Use the UIApplication lifecycle instead.
        // Terminating an app programmatically is strongly discouraged by Apple.
        // Combined with the plist setting "Application does not run in background" suspend allows
        // the XCUITest to wake up, so it doesn't need to guess how long we'll take and wait.
        UIApplication *app = [UIApplication sharedApplication];
        if ([app respondsToSelector:@selector(suspend)])
        {
            [app performSelector:@selector(suspend)];
        }
    }
}

- (void)mapDidFinishRenderingFrameFullyRendered:(__unused BOOL)fullyRendered
                              frameEncodingTime:(double)frameEncodingTime
                             frameRenderingTime:(double)frameRenderingTime
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [self renderFrame];
    });
    
    /*if (state == State::Benchmarking)
    {
        frames++;
        totalFrameEncodingTime += frameEncodingTime;
        totalFrameRenderingTime += frameRenderingTime;
        if (frames >= benchmarkDuration)
        {
            state = State::None;

            // Report FPS
            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - started).count();
            const auto wallClockFPS = double(frames * 1e6) / duration;
            const auto frameEncodingTime = static_cast<double>(totalFrameEncodingTime) / frames;
            const auto frameRenderingTime = static_cast<double>(totalFrameRenderingTime) / frames;
            result.emplace_back(mbgl::bench::locations[idx].name, std::make_pair(frameEncodingTime, frameRenderingTime));
            NSLog(@"- Frame encoding time: %.1f ms, Frame rendering time: %.1f ms (%.1f sync FPS)", frameEncodingTime * 1e3, frameRenderingTime * 1e3, wallClockFPS);

            // Start benchmarking the next location.
            idx++;
            [self startBenchmarkIteration];
        } else {
            dispatch_async(dispatch_get_main_queue(), ^{
                [self renderFrame];
            });
        }
        return;
    }
    else if (state == State::WarmingUp)
    {
        frames++;
        if (frames >= warmupDuration)
        {
            frames = 0;
            totalFrameEncodingTime = 0;
            totalFrameRenderingTime = 0;
            state = State::Benchmarking;
            started = std::chrono::steady_clock::now();
            NSLog(@"- Benchmarking for %d frames...", benchmarkDuration);
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            [self renderFrame];
        });
        return;
    }
    else if (state == State::WaitingForAssets)
    {
        if (map->isFullyLoaded())
        {
            // Start the benchmarking timer.
            state = State::WarmingUp;
            NSLog(@"- Warming up for %d frames...", warmupDuration);
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            [self renderFrame];
        });
        return;
    }*/
}

- (NSUInteger)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskLandscape;
}

@end
