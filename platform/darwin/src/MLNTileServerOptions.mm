#import "MLNTileServerOptions.h"
#import "NSBundle+MLNAdditions.h"

#if TARGET_OS_OSX
#import "NSProcessInfo+MLNAdditions.h"
#endif

@implementation MLNTileServerOptions

@synthesize baseURL;
@synthesize uriSchemeAlias;
@synthesize sourceTemplate;
@synthesize sourceDomainName;
@synthesize sourceVersionPrefix;
@synthesize styleTemplate;
@synthesize styleDomainName;
@synthesize styleVersionPrefix;
@synthesize spritesTemplate;
@synthesize spritesDomainName;
@synthesize spritesVersionPrefix;
@synthesize glyphsTemplate;
@synthesize glyphsDomainName;
@synthesize glyphsVersionPrefix;
@synthesize tileTemplate;
@synthesize tileDomainName;
@synthesize tileVersionPrefix;
@synthesize apiKeyParameterName;
@synthesize defaultStyles;
@synthesize defaultStyle;

@end
