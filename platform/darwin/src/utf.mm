#import <Foundation/Foundation.h>

#include <mbgl/util/utf.hpp>
#include <string>

namespace mbgl {
namespace util {

std::string convertUTF16ToUTF8(const std::u16string& str) {
    const NSString* original = [[NSString alloc] initWithBytesNoCopy:const_cast<char *>(str.data())
        length:str.size()
        encoding:NSUTF8StringEncoding
        freeWhenDone:NO];
    return std::u16string(reinterpret_cast<const char16_t*>([original cStringUsingEncoding:NSUTF16StringEncoding]),
                          [original length]);
}

std::string convertUTF16ToUTF8(const std::u16string& str) {
    static_assert(sizeof(unichar) == sizeof(std::u16string::value_type));
    return [[NSString stringWithCharacters:reinterpret_cast<const unichar*>(str.c_str()) length:str.size()] UTF8String];
}

} // namespace util
} // namespace mbgl
