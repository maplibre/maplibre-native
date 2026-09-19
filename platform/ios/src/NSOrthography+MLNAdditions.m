#import "NSOrthography+MLNAdditions.h"

@implementation NSOrthography (MLNAdditions)

+ (NSString *)mgl_dominantScriptForMapboxStreetsLanguage:(NSString *)language {
    NSLocale *locale = [NSLocale localeWithLocaleIdentifier:language];
    NSOrthography *orthography = [NSOrthography defaultOrthographyForLanguage:locale.localeIdentifier];
    return orthography.dominantScript;
}

@end
