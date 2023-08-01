#import <Cocoa/Cocoa.h>

@class MLNMapView;

@interface MapDocument : NSDocument

@property (weak) IBOutlet MLNMapView *mapView;

- (IBAction)showStyle:(id)sender;
- (IBAction)chooseCustomStyle:(id)sender;

- (IBAction)reload:(id)sender;

@end
