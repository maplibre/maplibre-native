#include <mln/util/enum.hpp>
#include <mln/util/logging.hpp>

#import <Foundation/Foundation.h>

namespace mln {

void Log::platformRecord(EventSeverity severity, const std::string &msg) {
  NSString *message = [[NSString alloc] initWithBytes:msg.data()
                                               length:msg.size()
                                             encoding:NSUTF8StringEncoding];
  NSLog(@"[%s] %@", Enum<EventSeverity>::toString(severity), message);
}

}  // namespace mln
