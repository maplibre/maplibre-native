#import "MGLMapViewIntegrationTest.h"

@interface MGLStyleURLIntegrationTest : MGLMapViewIntegrationTest
@end

@implementation MGLStyleURLIntegrationTest

- (void)setUp {
    [super setUp];
    [MGLSettings useWellKnownTileServer:MGLMapTiler];
}

- (void)predefinedStylesLoadingTest {
    
    for (MGLDefaultStyle* style in [MGLStyle predefinedStyles]) {
        NSString* styleName = style.name;
        self.mapView.styleURL = [[MGLStyle predefinedStyle:styleName] url];
        [self waitForMapViewToFinishLoadingStyleWithTimeout:5];
    }
}

@end
