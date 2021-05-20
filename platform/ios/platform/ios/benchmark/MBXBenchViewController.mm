#import "MBXBenchViewController.h"
#import "MBXBenchAppDelegate.h"
#import "MGLMapView_Private.h"
#import "MGLMapViewDelegate.h"

#include "locations.hpp"

#include <chrono>

@interface MBXBenchViewController () <MGLMapViewDelegate>

@property (nonatomic) MGLMapView *mapView;

@end

@implementation MBXBenchViewController

#pragma mark - Setup

+ (void)initialize
{
    if (self == [MBXBenchViewController class])
    {
        [[NSUserDefaults standardUserDefaults] registerDefaults:@{
            @"MBXUserTrackingMode": @(MGLUserTrackingModeNone),
            @"MBXShowsUserLocation": @NO,
            @"MBXDebug": @NO,
        }];
    }
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    // Use a local style and local assets if they’ve been downloaded.
    NSURL *tile = [[NSBundle mainBundle] URLForResource:@"11" withExtension:@"pbf" subdirectory:@"tiles/tiles/v3/5/7"];
    NSURL *tileSourceURL = [[NSBundle mainBundle] URLForResource:@"openmaptiles" withExtension:@"json" subdirectory:@"tiles"];
    NSURL *url = [NSURL URLWithString:tile ? @"asset://styles/streets.json" : @"maptiler://maps/streets"];
    self.mapView = [[MGLMapView alloc] initWithFrame:self.view.bounds styleURL:url];
    self.mapView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    self.mapView.delegate = self;
    self.mapView.zoomEnabled = NO;
    self.mapView.scrollEnabled = NO;
    self.mapView.rotateEnabled = NO;
    self.mapView.userInteractionEnabled = YES;
    self.mapView.preferredFramesPerSecond = MGLMapViewPreferredFramesPerSecondMaximum;

    [self.view addSubview:self.mapView];
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    
    #ifdef LOG_TO_DOCUMENTS_DIR
    [self setAndRedirectLogFileToDocuments];
    #endif

    [self startBenchmarkIteration];
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
std::chrono::steady_clock::time_point started;
std::vector<std::pair<std::string, double>> result;

static const int warmupDuration = 20; // frames
static const int benchmarkDuration = 200; // frames

- (void)startBenchmarkIteration
{
    if (mbgl::bench::locations.size() > idx) {
        const auto& location = mbgl::bench::locations[idx];
        [self.mapView setCenterCoordinate:CLLocationCoordinate2DMake(location.latitude, location.longitude) zoomLevel:location.zoom animated:NO];
        self.mapView.direction = location.bearing;
        state = State::WaitingForAssets;
        NSLog(@"Benchmarking \"%s\"", location.name.c_str());
        NSLog(@"- Loading assets...");
    } else {
        // Do nothing. The benchmark is completed.
        NSLog(@"Benchmark completed.");
        NSLog(@"Result:");
        double totalFPS = 0;
        size_t colWidth = 0;
        for (const auto& row : result) {
            colWidth = std::max(row.first.size(), colWidth);
        }
        for (const auto& row : result) {
            NSLog(@"| %-*s | %4.1f fps |", int(colWidth), row.first.c_str(), row.second);
            totalFPS += row.second;
        }
        NSLog(@"Total FPS: %4.1f", totalFPS);
        NSLog(@"Average FPS: %4.1f", totalFPS / result.size());
        exit(0);
    }
}

- (void)mapViewDidFinishRenderingFrame:(MGLMapView *)mapView fullyRendered:(__unused BOOL)fullyRendered
{
    if (state == State::Benchmarking)
    {
        frames++;
        if (frames >= benchmarkDuration)
        {
            state = State::None;

            // Report FPS
            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - started).count() ;
            const auto fps = double(frames * 1e6) / duration;
            result.emplace_back(mbgl::bench::locations[idx].name, fps);
            NSLog(@"- FPS: %.1f", fps);

            // Start benchmarking the next location.
            idx++;
            [self startBenchmarkIteration];
        } else {
            [mapView setNeedsRerender];
        }
        return;
    }

    else if (state == State::WarmingUp)
    {
        frames++;
        if (frames >= warmupDuration)
        {
            frames = 0;
            state = State::Benchmarking;
            started = std::chrono::steady_clock::now();
            NSLog(@"- Benchmarking for %d frames...", benchmarkDuration);
        }
        [mapView setNeedsRerender];
        return;
    }

    else if (state == State::WaitingForAssets)
    {
        if ([mapView isFullyLoaded])
        {
            // Start the benchmarking timer.
            state = State::WarmingUp;
            [self.mapView didReceiveMemoryWarning];
            NSLog(@"- Warming up for %d frames...", warmupDuration);
            [mapView setNeedsRerender];
        }
        return;
    }
}

- (NSUInteger)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskLandscape;
}

@end
