#import "MLNAttributionInfo_Private.h"

#if TARGET_OS_IPHONE
    #import <UIKit/UIKit.h>
#else
    #import <Cocoa/Cocoa.h>
#endif

#import "MLNSettings.h"
#import "MLNMapCamera.h"
#import "NSArray+MLNAdditions.h"
#import "NSBundle+MLNAdditions.h"
#import "NSString+MLNAdditions.h"
#import "MLNLoggingConfiguration_Private.h"

#include <string>

@implementation MLNAttributionInfo

+ (NSArray<MLNAttributionInfo *> *)attributionInfosFromHTMLString:(nullable NSString *)htmlString fontSize:(CGFloat)fontSize linkColor:(nullable MLNColor *)linkColor {
    if (!htmlString) {
        return @[];
    }

    NSDictionary *options = @{
        NSDocumentTypeDocumentAttribute: NSHTMLTextDocumentType,
        NSCharacterEncodingDocumentAttribute: @(NSUTF8StringEncoding),
    };
    // Apply a bogus, easily detectable style rule to any feedback link, since
    // NSAttributedString doesn’t preserve the class attribute.
    NSMutableString *css = [NSMutableString stringWithString:
                            @"html { font-family: -apple-system, -apple-system-font, sans-serif; }"
                            @".mapbox-improve-map { -webkit-text-stroke-width: 1000px; }"];
    if (fontSize) {
        NSString *sizeRule = [NSString stringWithFormat:@"font-size: %.1fpx;", fontSize];
#if !TARGET_OS_IPHONE
        if (fontSize == [NSFont systemFontSizeForControlSize:NSControlSizeMini]) {
            sizeRule = @"font: -webkit-mini-control";
        } else if (fontSize == [NSFont systemFontSizeForControlSize:NSControlSizeSmall]) {
            sizeRule = @"font: -webkit-small-control";
        } else if (fontSize == [NSFont systemFontSizeForControlSize:NSControlSizeRegular]) {
            sizeRule = @"font: -webkit-control";
        }
#endif
        [css appendFormat:@"html { %@ }", sizeRule];
    }
    if (linkColor) {
        CGFloat red;
        CGFloat green;
        CGFloat blue;
        CGFloat alpha;
#if !TARGET_OS_IPHONE
        // CSS uses the sRGB color space.
        if ([NSColor redColor].colorSpaceName == NSCalibratedRGBColorSpace) {
            linkColor = [linkColor colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
        } else {
            linkColor = [linkColor colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];
        }
#endif
        [linkColor getRed:&red green:&green blue:&blue alpha:&alpha];
        [css appendFormat:
         @"a:link { color: rgba(%f%%, %f%%, %f%%, %f); }",
         red * 100, green * 100, blue * 100, alpha];
    }
    NSString *styledHTML = [NSString stringWithFormat:@"<style type='text/css'>%@</style>%@", css, htmlString];
    NSData *htmlData = [styledHTML dataUsingEncoding:NSUTF8StringEncoding];

#if TARGET_OS_IPHONE
    __block NSMutableAttributedString *attributedString;
    dispatch_block_t initialization = ^{
            // This initializer should be called from a global or main queue. https://developer.apple.com/documentation/foundation/nsattributedstring/1524613-initwithdata
            attributedString = [[NSMutableAttributedString alloc] initWithData:htmlData
                                                                       options:options
                                                            documentAttributes:nil
                                                                         error:NULL];
    };
    
    if (![[NSThread currentThread] isMainThread]) {
        dispatch_sync(dispatch_get_main_queue(), initialization);
    } else {
        initialization();
    }
#else
    NSMutableAttributedString *attributedString = [[NSMutableAttributedString alloc] initWithHTML:htmlData
                                                                                          options:options
                                                                               documentAttributes:nil];
#endif

    NSMutableArray *infos = [NSMutableArray array];
    [attributedString enumerateAttribute:NSLinkAttributeName
                                 inRange:attributedString.mgl_wholeRange
                                 options:0
                              usingBlock:
    ^(id _Nullable value, NSRange range, BOOL * _Nonnull stop) {
        MLNCAssert(!value || [value isKindOfClass:[NSURL class]], @"If present, URL attribute must be an NSURL.");

        // Detect feedback links by the bogus style rule applied above.
        NSNumber *strokeWidth = [attributedString attribute:NSStrokeWidthAttributeName
                                                    atIndex:range.location
                                             effectiveRange:NULL];
        BOOL isFeedbackLink = NO;
        if ([strokeWidth floatValue] > 100) {
            isFeedbackLink = YES;
            [attributedString removeAttribute:NSStrokeWidthAttributeName range:range];
        }

        // Omit whitespace-only strings.
        NSAttributedString *title = [[attributedString attributedSubstringFromRange:range]
                                     mgl_attributedStringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
        if (!title.length) {
            return;
        }

        // Remove the link, because it forces the text to be blue on macOS 10.12
        // and above.
        NSMutableAttributedString *unlinkedTitle = [title mutableCopy];
        [unlinkedTitle removeAttribute:NSLinkAttributeName range:unlinkedTitle.mgl_wholeRange];
        
        MLNAttributionInfo *info = [[MLNAttributionInfo alloc] initWithTitle:unlinkedTitle URL:value];
        info.feedbackLink = isFeedbackLink;
        [infos addObject:info];
    }];
    return infos;
}

+ (NSAttributedString *)attributedStringForAttributionInfos:(NSArray<MLNAttributionInfo *> *)attributionInfos {
    NSMutableArray *titles = [NSMutableArray arrayWithCapacity:attributionInfos.count];
    for (MLNAttributionInfo *info in attributionInfos) {
        NSMutableAttributedString *title = info.title.mutableCopy;
        if (info.URL) {
            [title addAttribute:NSLinkAttributeName value:info.URL range:title.mgl_wholeRange];
        }
        [titles addObject:title];
    }
    return [titles mgl_attributedComponentsJoinedByString:@" "];
}

- (instancetype)initWithTitle:(NSAttributedString *)title URL:(NSURL *)URL {
    if (self = [super init]) {
        _title = title;
        _URL = URL;
    }
    return self;
}

- (id)copyWithZone:(nullable NSZone *)zone
{
    MLNAttributionInfo *info = [[[self class] allocWithZone:zone] initWithTitle:_title
                                                                            URL:_URL];
    info.feedbackLink = _feedbackLink;
    
    return info;
}

- (NSAttributedString *)titleWithStyle:(MLNAttributionInfoStyle)style
{
    NSString *openStreetMap = NSLocalizedStringWithDefaultValue(@"OSM_FULL_NAME", @"Foundation", nil, @"OpenStreetMap", @"OpenStreetMap full name attribution");
    NSString *OSM = NSLocalizedStringWithDefaultValue(@"OSM_SHORT_NAME", @"Foundation", nil, @"OSM", @"OpenStreetMap short name attribution");
    
    NSMutableAttributedString *title = [[NSMutableAttributedString alloc] initWithAttributedString:self.title];
    [title removeAttribute:NSUnderlineStyleAttributeName range:NSMakeRange(0, [title.string length])];
    
    BOOL isAbbreviated = (style == MLNAttributionInfoStyleShort);
    
    if ([title.string rangeOfString:@"OpenStreetMap"].location != NSNotFound) {
        [title.mutableString replaceOccurrencesOfString:@"OpenStreetMap" withString:isAbbreviated ? OSM : openStreetMap
                                                options:NSCaseInsensitiveSearch
                                                  range:NSMakeRange(0, [title.mutableString length])];
    }
    
    return title;
}

- (BOOL)isEqual:(id)object {
    return [object isKindOfClass:[self class]] && [[object title] isEqual:self.title] && [[object URL] isEqual:self.URL];
}

- (NSUInteger)hash {
    return self.title.hash + self.URL.hash;
}

/**
 Returns whether the given attribution info object overlaps with the receiver by
 its plain text title.

 @return `NSOrderedAscending` if the given object is a superset of the receiver,
    `NSOrderedDescending` if it is a subset of the receiver, or `NSOrderedSame`
    if there is no overlap.
 */
- (NSComparisonResult)subsetCompare:(MLNAttributionInfo *)otherInfo {
    NSString *title = self.title.string;
    NSString *otherTitle = otherInfo.title.string;
    if ([title containsString:otherTitle]) {
        return NSOrderedDescending;
    }
    if ([otherTitle containsString:title]) {
        return NSOrderedAscending;
    }
    return NSOrderedSame;
}

@end

@implementation NSMutableArray (MLNAttributionInfoAdditions)

- (void)growArrayByAddingAttributionInfo:(MLNAttributionInfo *)info {
    __block BOOL didInsertInfo = NO;
    __block BOOL shouldAddInfo = YES;
    [self enumerateObjectsUsingBlock:^(MLNAttributionInfo * _Nonnull existingInfo, NSUInteger idx, BOOL * _Nonnull stop) {
        switch ([info subsetCompare:existingInfo]) {
            case NSOrderedDescending:
                // The existing info object is a subset of the one we’re adding.
                // Replace the existing object the first time we find a subset;
                // remove the existing object every time after that.
                if (didInsertInfo) {
                    [self removeObjectAtIndex:idx];
                } else {
                    [self replaceObjectAtIndex:idx withObject:info];
                    didInsertInfo = YES;
                }
                break;

            case NSOrderedAscending:
                // The info object we’re adding is a subset of the existing one.
                // Don’t add the object and stop looking.
                shouldAddInfo = NO;
                *stop = YES;
                break;

            default:
                break;
        }
    }];
    if (shouldAddInfo && !didInsertInfo) {
        // No overlapping infos were found, so append the info object.
        [self addObject:info];
    }
}

- (void)growArrayByAddingAttributionInfosFromArray:(NSArray<MLNAttributionInfo *> *)infos {
    for (MLNAttributionInfo *info in infos) {
        [self growArrayByAddingAttributionInfo:info];
    }
}

@end
