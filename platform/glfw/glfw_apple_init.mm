#import <Foundation/Foundation.h>

// Called early in GLFW initialization so that the local glyph rasterizer
// can find a suitable CJK font via NSUserDefaults.
__attribute__((constructor))
static void setDefaultIdeographicFont() {
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    if (![defaults objectForKey:@"MLNIdeographicFontFamilyName"]) {
        [defaults setObject:@[@"PingFang TC"] forKey:@"MLNIdeographicFontFamilyName"];
    }
}
