#import "MLNMapViewIntegrationTest.h"

@interface MLNStyleURLIntegrationTest : MLNMapViewIntegrationTest
@end

@implementation MLNStyleURLIntegrationTest

- (void)setUp {
    [super setUp];
    [MLNSettings useWellKnownTileServer:MLNMapTiler];
}

- (void)predefinedStylesLoadingTest {
    
    for (MLNDefaultStyle* style in [MLNStyle predefinedStyles]) {
        NSString* styleName = style.name;
        self.mapView.styleURL = [[MLNStyle predefinedStyle:styleName] url];
        [self waitForMapViewToFinishLoadingStyleWithTimeout:5];
    }
}

@end
