#import "MBXSnapshotTestViewController.h"
#import "Mapbox.h"
#import "MLNMapView_Private.h"
#import "MLNMapView+Impl.h"

@interface MBXSnapshotTestViewController () <MLNMapViewDelegate>
@property (nonatomic) UIImageView *snapshotImageView;
@property (nonatomic, weak) id<MLNMapViewDelegate> originalDelegate;
@property (nonatomic, weak) UIView *originalSuperview;
@property (nonatomic) NSArray<NSLayoutConstraint *> *mapConstraints;
@end

@implementation MBXSnapshotTestViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.title = @"Live Snapshot";
    self.view.backgroundColor = [UIColor blackColor];

    _snapshotImageView = [[UIImageView alloc] init];
    _snapshotImageView.contentMode = UIViewContentModeScaleAspectFit;
    _snapshotImageView.backgroundColor = [UIColor darkGrayColor];
    _snapshotImageView.translatesAutoresizingMaskIntoConstraints = NO;
    [self.view addSubview:_snapshotImageView];

    UILayoutGuide *safe = self.view.safeAreaLayoutGuide;

    if (_sourceMapView) {
        _originalDelegate = _sourceMapView.delegate;
        _originalSuperview = _sourceMapView.superview;

        _sourceMapView.delegate = self;
        _sourceMapView.translatesAutoresizingMaskIntoConstraints = NO;
        [self.view addSubview:_sourceMapView];

        _mapConstraints = @[
            [_snapshotImageView.topAnchor constraintEqualToAnchor:safe.topAnchor],
            [_snapshotImageView.leadingAnchor constraintEqualToAnchor:safe.leadingAnchor],
            [_snapshotImageView.trailingAnchor constraintEqualToAnchor:safe.trailingAnchor],
            [_snapshotImageView.heightAnchor constraintEqualToAnchor:safe.heightAnchor multiplier:0.5],

            [_sourceMapView.topAnchor constraintEqualToAnchor:_snapshotImageView.bottomAnchor],
            [_sourceMapView.leadingAnchor constraintEqualToAnchor:safe.leadingAnchor],
            [_sourceMapView.trailingAnchor constraintEqualToAnchor:safe.trailingAnchor],
            [_sourceMapView.bottomAnchor constraintEqualToAnchor:safe.bottomAnchor],
        ];
        [NSLayoutConstraint activateConstraints:_mapConstraints];
    } else {
        [NSLayoutConstraint activateConstraints:@[
            [_snapshotImageView.topAnchor constraintEqualToAnchor:safe.topAnchor],
            [_snapshotImageView.leadingAnchor constraintEqualToAnchor:safe.leadingAnchor],
            [_snapshotImageView.trailingAnchor constraintEqualToAnchor:safe.trailingAnchor],
            [_snapshotImageView.bottomAnchor constraintEqualToAnchor:safe.bottomAnchor],
        ]];
    }
}

- (void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];

    if (self.isMovingFromParentViewController && _sourceMapView && _originalSuperview) {
        _sourceMapView.delegate = _originalDelegate;

        if (_mapConstraints) {
            [NSLayoutConstraint deactivateConstraints:_mapConstraints];
            _mapConstraints = nil;
        }

        // Re-pin to all edges; original storyboard constraints were
        // removed when the map view was reparented into this VC.
        [_originalSuperview addSubview:_sourceMapView];
        _sourceMapView.translatesAutoresizingMaskIntoConstraints = NO;
        [NSLayoutConstraint activateConstraints:@[
            [_sourceMapView.topAnchor constraintEqualToAnchor:_originalSuperview.topAnchor],
            [_sourceMapView.leadingAnchor constraintEqualToAnchor:_originalSuperview.leadingAnchor],
            [_sourceMapView.trailingAnchor constraintEqualToAnchor:_originalSuperview.trailingAnchor],
            [_sourceMapView.bottomAnchor constraintEqualToAnchor:_originalSuperview.bottomAnchor],
        ]];
    }
}

- (void)takeSnapshot {
    if (!_sourceMapView) return;
    UIImage *image = [_sourceMapView viewImpl]->snapshot();
    if (image) {
        self.snapshotImageView.image = image;
    }
}

#pragma mark - MLNMapViewDelegate

- (void)mapViewDidFinishRenderingFrame:(MLNMapView *)mapView fullyRendered:(BOOL)fullyRendered {
    [self takeSnapshot];
}

@end
