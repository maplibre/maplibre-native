#import "Mapbox.h"

#import "MBXOrnamentsViewController.h"

@interface MBXOrnamentsViewController ()

@property (nonatomic) MLNMapView *mapView;
@property (nonatomic) NSTimer *timer;
@property (nonatomic) NSInteger currentPositionIndex;

@end

@implementation MBXOrnamentsViewController

- (void)setCurrentPositionIndex:(NSInteger)currentPositionIndex {
    MLNOrnamentPosition ornamentPositions[5][4] = {
        {
            MLNOrnamentPositionTopLeft,
            MLNOrnamentPositionTopRight,
            MLNOrnamentPositionBottomRight,
            MLNOrnamentPositionBottomLeft
        },
        {
            MLNOrnamentPositionTopRight,
            MLNOrnamentPositionBottomRight,
            MLNOrnamentPositionBottomLeft,
            MLNOrnamentPositionTopLeft
        },
        {
            MLNOrnamentPositionBottomRight,
            MLNOrnamentPositionBottomLeft,
            MLNOrnamentPositionTopLeft,
            MLNOrnamentPositionTopRight
        },
        {
            MLNOrnamentPositionBottomLeft,
            MLNOrnamentPositionTopLeft,
            MLNOrnamentPositionTopRight,
            MLNOrnamentPositionBottomRight
        },
        {
            MLNOrnamentPositionTopLeft,
            MLNOrnamentPositionTopRight,
            MLNOrnamentPositionBottomRight,
            MLNOrnamentPositionBottomLeft
        }
    };
    MLNOrnamentPosition *currentPosition = ornamentPositions[currentPositionIndex];
    self.mapView.scaleBarPosition = currentPosition[0];
    self.mapView.compassViewPosition = currentPosition[1];
    self.mapView.logoViewPosition = currentPosition[2];
    self.mapView.attributionButtonPosition = currentPosition[3];
    
    _currentPositionIndex = currentPositionIndex;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    self.title = @"Ornaments";

    MLNMapView *mapView = [[MLNMapView alloc] initWithFrame:self.view.bounds];
    mapView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [mapView setCenterCoordinate:CLLocationCoordinate2DMake(39.915143, 116.404053)
                       zoomLevel:16
                       direction:30
                        animated:NO];
    mapView.showsScale = YES;
    [self.view addSubview:mapView];

    self.mapView = mapView;
}

- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidDisappear:animated];
    [self.timer invalidate];
    self.timer = nil;
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    self.timer = [NSTimer scheduledTimerWithTimeInterval:1
                                                  target:self
                                                selector:@selector(onTimerTick)
                                                userInfo:nil
                                                 repeats:YES];
}

- (void)onTimerTick {
    self.currentPositionIndex ++;
    if (self.currentPositionIndex >= 4) {
        self.currentPositionIndex = 0;
    }
}

@end
