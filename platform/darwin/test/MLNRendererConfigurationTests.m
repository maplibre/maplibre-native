#import <Mapbox.h>
#import <XCTest/XCTest.h>
#import "MLNRendererConfiguration.h"

static NSString * const MLNRendererConfigurationTests_collisionBehaviorKey = @"MLNCollisionBehaviorPre4_0";

@interface MLNRendererConfigurationTests : XCTestCase
@end

@implementation MLNRendererConfigurationTests
- (void)setUp {
    [[NSUserDefaults standardUserDefaults] removeObjectForKey:MLNRendererConfigurationTests_collisionBehaviorKey];
}

- (void)tearDown {
    [[NSUserDefaults standardUserDefaults] removeObjectForKey:MLNRendererConfigurationTests_collisionBehaviorKey];
}

// Emulate what would happen with an Info.plist.
- (void)testSettingMLNCollisionBehaviorPre40
{
    MLNRendererConfiguration *config = [[MLNRendererConfiguration alloc] init];
    XCTAssertFalse([config perSourceCollisionsWithInfoDictionaryObject:nil]);
    XCTAssertFalse([config perSourceCollisionsWithInfoDictionaryObject:@(NO)]);
    XCTAssertTrue([config perSourceCollisionsWithInfoDictionaryObject:@(YES)]);
    XCTAssertFalse([config perSourceCollisionsWithInfoDictionaryObject:@"NO"]);
    XCTAssertTrue([config perSourceCollisionsWithInfoDictionaryObject:@"YES"]);
}

- (void)testSettingMLNCollisionBehaviorPre40InNSUserDefaults {
    {
        XCTAssertNil([[NSUserDefaults standardUserDefaults] objectForKey:MLNRendererConfigurationTests_collisionBehaviorKey]);
        MLNRendererConfiguration *config = [MLNRendererConfiguration currentConfiguration];
        XCTAssertFalse(config.perSourceCollisions);
        XCTAssertFalse([config perSourceCollisionsWithInfoDictionaryObject:nil]);
    }
    
    [[NSUserDefaults standardUserDefaults] setObject:@(NO) forKey:MLNRendererConfigurationTests_collisionBehaviorKey];
    {
        XCTAssertNotNil([[NSUserDefaults standardUserDefaults] objectForKey:MLNRendererConfigurationTests_collisionBehaviorKey]);
        MLNRendererConfiguration *config = [MLNRendererConfiguration currentConfiguration];
        XCTAssertFalse(config.perSourceCollisions);
        XCTAssertFalse([config perSourceCollisionsWithInfoDictionaryObject:@(NO)]);
        XCTAssertFalse([config perSourceCollisionsWithInfoDictionaryObject:@(YES)]);
    }
    
    [[NSUserDefaults standardUserDefaults] setObject:@(YES) forKey:MLNRendererConfigurationTests_collisionBehaviorKey];
    {
        XCTAssertNotNil([[NSUserDefaults standardUserDefaults] objectForKey:MLNRendererConfigurationTests_collisionBehaviorKey]);
        MLNRendererConfiguration *config = [MLNRendererConfiguration currentConfiguration];
        XCTAssert(config.perSourceCollisions);
        XCTAssertTrue([config perSourceCollisionsWithInfoDictionaryObject:@(NO)]);
        XCTAssertTrue([config perSourceCollisionsWithInfoDictionaryObject:@(YES)]);
    }
}

- (void)testOverridingMLNCollisionBehaviorPre40 {
    // Dictionary = NO, NSUserDefaults = YES
    {
        [[NSUserDefaults standardUserDefaults] setObject:@(YES) forKey:MLNRendererConfigurationTests_collisionBehaviorKey];
        MLNRendererConfiguration *config = [[MLNRendererConfiguration alloc] init];
        XCTAssert([config perSourceCollisionsWithInfoDictionaryObject:@(NO)]);
    }
    // Dictionary = YES, NSUserDefaults = NO
    {
        [[NSUserDefaults standardUserDefaults] setObject:@(NO) forKey:MLNRendererConfigurationTests_collisionBehaviorKey];
        MLNRendererConfiguration *config = [[MLNRendererConfiguration alloc] init];
        XCTAssertFalse([config perSourceCollisionsWithInfoDictionaryObject:@(YES)]);
    }
}

- (void)testDefaultLocalFontFamilyName {
    
    MLNRendererConfiguration *config = [[MLNRendererConfiguration alloc] init];
    NSString *localFontFamilyName = config.localFontFamilyName;
    
    NSString *systemFontFamilyName;
#if TARGET_OS_IPHONE
    systemFontFamilyName = [UIFont systemFontOfSize:0 weight:UIFontWeightRegular].familyName;
#else
    systemFontFamilyName = [NSFont systemFontOfSize:0 weight:NSFontWeightRegular].familyName;
#endif
    
    XCTAssertEqualObjects(localFontFamilyName, systemFontFamilyName, @"Default local font family name should match default system font");
}

- (void)testSettingMLNIdeographicFontFamilyNameWithPlistValue {
    
    MLNRendererConfiguration *config = [[MLNRendererConfiguration alloc] init];
    
    // `MLNIdeographicFontFamilyName` set to bool value `YES`
    {
        NSString *localFontFamilyName = [config localFontFamilyNameWithInfoDictionaryObject:@(YES)];
        
        NSString *systemFontFamilyName;
#if TARGET_OS_IPHONE
        systemFontFamilyName = [UIFont systemFontOfSize:0 weight:UIFontWeightRegular].familyName;
#else
        systemFontFamilyName = [NSFont systemFontOfSize:0 weight:NSFontWeightRegular].familyName;
#endif
        XCTAssertEqualObjects(localFontFamilyName, systemFontFamilyName, @"Local font family name should match default system font name when setting `YES`");
    }
    
    // `MLNIdeographicFontFamilyName` set to bool value `NO`
    {
        NSString *localFontFamilyName = [config localFontFamilyNameWithInfoDictionaryObject:@(NO)];
        XCTAssertNil(localFontFamilyName, @"Client rendering font should use remote font when setting `NO`");
    }
    
    // `MLNIdeographicFontFamilyName` set to a valid font string value
    {
        NSString *localFontFamilyName = [config localFontFamilyNameWithInfoDictionaryObject:@"PingFang TC"];
        XCTAssertEqualObjects(localFontFamilyName, @"PingFang TC", @"Local font family name should match a custom valid font name");
    }
    
    // `MLNIdeographicFontFamilyName` set to an invalid font string value
    {
        NSString *localFontFamilyName = [config localFontFamilyNameWithInfoDictionaryObject:@"test font"];
        
        NSString *systemFontFamilyName;
#if TARGET_OS_IPHONE
        systemFontFamilyName = [UIFont systemFontOfSize:0 weight:UIFontWeightRegular].familyName;
#else
        systemFontFamilyName = [NSFont systemFontOfSize:0 weight:NSFontWeightRegular].familyName;
#endif
        XCTAssertNotEqualObjects(localFontFamilyName, systemFontFamilyName, @"Local font family name should not be validated by MLNRenderConfiguration");
    }
    
    // `MLNIdeographicFontFamilyName` set to a valid font family names array value
    {
        NSString *localFontFamilyName = [config localFontFamilyNameWithInfoDictionaryObject:@[@"test font 1", @"PingFang TC", @"test font 2"]];
        XCTAssertEqualObjects(localFontFamilyName, @"test font 1\nPingFang TC\ntest font 2");
    }
    
    // `MLNIdeographicFontFamilyName` set to an invalid font family names array value
    {
        NSString *localFontFamilyName = [config localFontFamilyNameWithInfoDictionaryObject:@[@"test font 1", @"test font 2", @"test font 3"]];

        XCTAssertEqualObjects(localFontFamilyName, @"test font 1\ntest font 2\ntest font 3", @"Local font family name should not be validated by MLNRendererConfiguration");
    }
    
    // `MLNIdeographicFontFamilyName` set to an invalid value type: NSDictionary, NSNumber, NSData, etc.
    {
        NSString *localFontFamilyName = [config localFontFamilyNameWithInfoDictionaryObject:[@"test font 1" dataUsingEncoding:NSUTF8StringEncoding]];
        
        NSString *systemFontFamilyName;
#if TARGET_OS_IPHONE
        systemFontFamilyName = [UIFont systemFontOfSize:0 weight:UIFontWeightRegular].familyName;
#else
        systemFontFamilyName = [NSFont systemFontOfSize:0 weight:NSFontWeightRegular].familyName;
#endif
        XCTAssertEqualObjects(localFontFamilyName, systemFontFamilyName, @"Local font family name should match default system font name when setting an invalid value type");
    }
}

@end
