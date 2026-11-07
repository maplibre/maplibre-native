#import <Foundation/Foundation.h>

@interface NSOrthography (NSOrthography_MLNAdditions)

/**
 Returns a four-letter ISO 15924 code representing the name of the dominant
 script for a given language.

 This method wraps `+[NSOrthography defaultOrthographyForLanguage:]` and
 supports any language.

 @param language The ISO-639 code representing a language.
 */
+ (NSString *)mgl_dominantScriptForMapboxStreetsLanguage:(NSString *)language;

@end
