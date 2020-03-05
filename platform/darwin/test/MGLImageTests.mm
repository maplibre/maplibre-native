#import <XCTest/XCTest.h>
#import <Mapbox/Mapbox.h>

#if TARGET_OS_IPHONE
    #import "UIImage+MGLAdditions.h"
#else
    #import "NSImage+MGLAdditions.h"
#endif

@interface MGLImageTests : XCTestCase

@end

@implementation MGLImageTests

- (void)testStretching {
#if TARGET_OS_IPHONE
    CGRect rect = CGRectMake(0, 0, 24, 24);
    UIGraphicsBeginImageContextWithOptions(rect.size, NO, UIScreen.mainScreen.scale);
    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextSetStrokeColorWithColor(context, UIColor.blackColor.CGColor);
    CGContextStrokeRectWithWidth(context, rect, 2);
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
#else
    NSImage *image = [NSImage imageWithSize:NSMakeSize(24, 24) flipped:NO drawingHandler:^BOOL(NSRect dstRect) {
        // A little more fanciful than the iOS version, but we aren’t testing the actual contents of the image anyways.
        NSRectEdge allSides[] = {NSMinYEdge, NSMaxXEdge, NSMaxYEdge, NSMinXEdge, NSMinYEdge, NSMaxXEdge};
        CGFloat grays[] = {NSBlack, NSBlack, NSWhite, NSWhite, NSDarkGray, NSDarkGray};
        dstRect = NSDrawTiledRects(dstRect, dstRect, allSides, grays, sizeof(grays) / sizeof(grays[0]));
        [NSColor.grayColor set];
        NSRectFill(dstRect);
        return YES;
    }];
#endif
    
    {
        auto styleImage = [image mgl_styleImageWithIdentifier:@"box"];
        XCTAssert(styleImage);
        if (styleImage) {
            XCTAssert(!styleImage->getContent());
            XCTAssertFalse(styleImage->isSdf());
            
            MGLImage *imageAfter = [[MGLImage alloc] initWithMGLStyleImage:*styleImage];
            XCTAssertEqual(imageAfter.capInsets.top, 0);
            XCTAssertEqual(imageAfter.capInsets.left, 0);
            XCTAssertEqual(imageAfter.capInsets.bottom, 0);
            XCTAssertEqual(imageAfter.capInsets.right, 0);
        }
    }
    
#if TARGET_OS_IPHONE
    image = [image resizableImageWithCapInsets:UIEdgeInsetsZero];
#else
    image.capInsets = NSEdgeInsetsZero;
#endif
    {
        auto styleImage = [image mgl_styleImageWithIdentifier:@"box"];
        XCTAssert(styleImage);
        if (styleImage) {
            XCTAssert(!styleImage->getContent());
            
            MGLImage *imageAfter = [[MGLImage alloc] initWithMGLStyleImage:*styleImage];
            XCTAssertEqual(imageAfter.capInsets.top, 0);
            XCTAssertEqual(imageAfter.capInsets.left, 0);
            XCTAssertEqual(imageAfter.capInsets.bottom, 0);
            XCTAssertEqual(imageAfter.capInsets.right, 0);
        }
    }
    
#if TARGET_OS_IPHONE
    image = [image resizableImageWithCapInsets:UIEdgeInsetsMake(1, 2, 3, 4)];
#else
    image.capInsets = NSEdgeInsetsMake(1, 2, 3, 4);
#endif
    {
        auto styleImage = [image mgl_styleImageWithIdentifier:@"box"];
        XCTAssert(styleImage);
        if (styleImage) {
            auto scale = styleImage->getPixelRatio();
            auto content = styleImage->getContent();
            XCTAssert(content);
            if (content) {
                XCTAssertEqual(content->top, 1 * scale);
                XCTAssertEqual(content->left, 2 * scale);
                XCTAssertEqual(content->bottom, 21 * scale);
                XCTAssertEqual(content->right, 20 * scale);
            }
            
            MGLImage *imageAfter = [[MGLImage alloc] initWithMGLStyleImage:*styleImage];
            XCTAssertEqual(imageAfter.capInsets.top, 1);
            XCTAssertEqual(imageAfter.capInsets.left, 2);
            XCTAssertEqual(imageAfter.capInsets.bottom, 3);
            XCTAssertEqual(imageAfter.capInsets.right, 4);
        }
    }
}

@end
