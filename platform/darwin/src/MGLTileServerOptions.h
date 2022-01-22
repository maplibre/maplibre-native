#import <Foundation/Foundation.h>

#import "MGLFoundation.h"
#import "MGLDefaultStyle.h"

NS_ASSUME_NONNULL_BEGIN

/**
 The `MGLTileServerOptions` defines server properties such as URL etc...
 */
MGL_EXPORT
@interface MGLTileServerOptions : NSObject

/**
* API base URL.
*/
@property (nonatomic,retain) NSString* baseURL;
        
/**
* Tile server URI alias.
*/
@property (nonatomic,retain) NSString* uriSchemeAlias;

/**
* URL template for sources.
*/
@property (nonatomic,retain) NSString* sourceTemplate;

/**
* source domain name.
*/
@property (nonatomic,retain) NSString* sourceDomainName;
        
/**
* source version prefix
*/
@property (nonatomic,retain,nullable) NSString* sourceVersionPrefix;

/**
* URL template for style.
*/
@property (nonatomic,retain) NSString* styleTemplate;
        
/**
* style domain name.
*/
@property (nonatomic,retain) NSString* styleDomainName;
        
/**
* style version prefix
*/
@property (nonatomic,retain,nullable) NSString* styleVersionPrefix;

/**
* URL template for sprites.
*/
@property (nonatomic,retain) NSString* spritesTemplate;
        
/**
* sprites domain name.
*/
@property (nonatomic,retain) NSString* spritesDomainName;

/**
* version prefix
*/
@property (nonatomic,retain,nullable) NSString* spritesVersionPrefix;

/**
* URL template for glyphs.
*/
@property (nonatomic,retain) NSString* glyphsTemplate;

/**
* glyphs domain name.
*/
@property (nonatomic,retain) NSString* glyphsDomainName;

/**
* Glyphs version prefix
*/
@property (nonatomic,retain,nullable) NSString* glyphsVersionPrefix;

/**
* URL template for tiles.
*/
@property (nonatomic,retain) NSString* tileTemplate;

/**
* tile domain name.
*/
@property (nonatomic,retain) NSString* tileDomainName;

/**
* tile version prefix
*/
@property (nonatomic,retain,nullable) NSString* tileVersionPrefix;
        
/**
* API key parameter name.
*/
@property (nonatomic,retain) NSString* apiKeyParameterName;

/**
* Default styles.
*/
@property (nonatomic, retain) NSArray<MGLDefaultStyle*>* defaultStyles;

/**
 * Default style
 */
@property (nonatomic, retain) MGLDefaultStyle* defaultStyle;

@end

NS_ASSUME_NONNULL_END
