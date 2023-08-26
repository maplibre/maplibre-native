#import <XCTest/XCTest.h>

#import <Mapbox.h>

@interface MLNImageSourceTests : XCTestCase

@end

@implementation MLNImageSourceTests


- (void)testMLNImageSourceWithImageURL {

    MLNCoordinateQuad quad = { { 80, 37}, { 81, 37}, { 81, 39}, { 80, 39}};
    MLNImageSource *source = [[MLNImageSource alloc] initWithIdentifier:@"source-id" coordinateQuad:quad URL:[NSURL URLWithString:@"http://host/image.png"]];
    
    XCTAssertNotNil(source.URL);
    XCTAssertEqualObjects(source.URL.absoluteString, @"http://host/image.png");
    XCTAssertNil(source.image);
}

- (void)testMLNImageSourceWithImage {

    NSString *imageName = @"RadarImage";
#if TARGET_OS_IPHONE
    MLNImage *image = [MLNImage imageNamed:imageName
                                  inBundle:[NSBundle bundleForClass:[self class]]
             compatibleWithTraitCollection:nil];
#else
    MLNImage *image = [[NSBundle bundleForClass:[self class]] imageForResource:imageName];
#endif
    XCTAssertNotNil(image);

    MLNCoordinateQuad quad = { { 80, 37}, { 81, 37}, { 81, 39}, { 80, 39}};
    MLNImageSource *source = [[MLNImageSource alloc] initWithIdentifier:@"source-id" coordinateQuad:quad image:image];
    
    XCTAssertNotNil(source.image);
    XCTAssertEqualObjects(source.image, image);
    XCTAssertNil(source.URL);
}

@end
