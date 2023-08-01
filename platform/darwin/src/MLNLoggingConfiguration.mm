#include <mbgl/util/logging.hpp>
#include <mbgl/util/enum.hpp>

#import "MLNLoggingConfiguration_Private.h"

#ifndef MLN_LOGGING_DISABLED
#if __has_builtin(__builtin_os_log_format)
#import <os/log.h>
#endif

namespace mbgl {
    
class MLNCoreLoggingObserver : public Log :: Observer {
public:
    //Return true not print messages at core level, and filter at platform level.
    bool onRecord(EventSeverity severity, Event event, int64_t code, const std::string& msg) override{
        
        NSString *message = [NSString stringWithFormat:@"[event]:%s [code]:%lld [message]:%@", Enum<Event>::toString(event), code, [NSString stringWithCString:msg.c_str() encoding:NSUTF8StringEncoding]];
        switch (severity) {
            case EventSeverity::Debug:
                MLNLogDebug(message);
                break;
            case EventSeverity::Info:
                MLNLogInfo(message);
                break;
            case EventSeverity::Warning:
                MLNLogWarning(message);
                break;
            case EventSeverity::Error:
                MLNLogError(message);
                break;
        }
        return true;
    }
};
}

@implementation MLNLoggingConfiguration
{
    std::unique_ptr<mbgl::MLNCoreLoggingObserver> _coreLoggingObserver;
}

+ (instancetype)sharedConfiguration {
    static dispatch_once_t once;
    static id sharedConfiguration;
    dispatch_once(&once, ^{
        sharedConfiguration = [[self alloc] init];
        ((MLNLoggingConfiguration *)sharedConfiguration).handler = nil;
    });
    return sharedConfiguration;
}

- (id)init{
    if(self = [super init]){
        mbgl::Log::setObserver(std::make_unique<mbgl::MLNCoreLoggingObserver>());
    }
    return self;
}

- (void)setHandler:(void (^)(MLNLoggingLevel, NSString *, NSUInteger, NSString *))handler {
    
    if (!handler) {
        _handler = [self defaultBlockHandler];
    } else {
        _handler = handler;
    }
}

- (void)logCallingFunction:(const char *)callingFunction functionLine:(NSUInteger)functionLine messageType:(MLNLoggingLevel)type format:(id)messageFormat, ... {
    va_list formatList;
    va_start(formatList, messageFormat);
    NSString *formattedMessage = [[NSString alloc] initWithFormat:messageFormat arguments:formatList];
    va_end(formatList);
    
    _handler(type, @(callingFunction), functionLine, formattedMessage);
    
}

- (MLNLoggingBlockHandler)defaultBlockHandler {
    MLNLoggingBlockHandler mapboxHandler = ^(MLNLoggingLevel level, NSString *fileName, NSUInteger line, NSString *message) {
        
        if (@available(iOS 10.0, macOS 10.12.0, *)) {
            static dispatch_once_t once;
            static os_log_t info_log;
#if MLN_LOGGING_ENABLE_DEBUG
            static os_log_t debug_log;
#endif
            static os_log_t error_log;
            static os_log_t fault_log;
            static os_log_type_t log_types[] = { OS_LOG_TYPE_DEFAULT,
                                                    OS_LOG_TYPE_INFO,
#if MLN_LOGGING_ENABLE_DEBUG
                                                    OS_LOG_TYPE_DEBUG,
#endif
                                                    OS_LOG_TYPE_ERROR,
                                                    OS_LOG_TYPE_FAULT };
            dispatch_once(&once, ^ {
                info_log = os_log_create("com.mapbox.Mapbox", "INFO");
#if MLN_LOGGING_ENABLE_DEBUG
                debug_log = os_log_create("com.mapbox.Mapbox", "DEBUG");
#endif
                error_log = os_log_create("com.mapbox.Mapbox", "ERROR");
                fault_log = os_log_create("com.mapbox.Mapbox", "FAULT");
            });
            
            os_log_t mapbox_log;
            switch (level) {
                case MLNLoggingLevelInfo:
                case MLNLoggingLevelWarning:
                    mapbox_log = info_log;
                    break;
#if MLN_LOGGING_ENABLE_DEBUG
                case MLNLoggingLevelDebug:
                    mapbox_log = debug_log;
                    break;
#endif
                case MLNLoggingLevelError:
                    mapbox_log = error_log;
                    break;
                case MLNLoggingLevelFault:
                    mapbox_log = fault_log;
                    break;
                case MLNLoggingLevelNone:
                default:
                    break;
            }

            os_log_type_t logType = log_types[level];
            os_log_with_type(mapbox_log, logType, "%@ - %lu: %@", fileName, (unsigned long)line, message);
        } else {
            NSString *category;
            switch (level) {
                case MLNLoggingLevelInfo:
                case MLNLoggingLevelWarning:
                    category = @"INFO";
                    break;
#if MLN_LOGGING_ENABLE_DEBUG
                case MLNLoggingLevelDebug:
                    category = @"DEBUG";
                    break;
#endif
                case MLNLoggingLevelError:
                    category = @"ERROR";
                    break;
                case MLNLoggingLevelFault:
                    category = @"FAULT";
                    break;
                case MLNLoggingLevelNone:
                default:
                    break;
            }
            
            NSLog(@"[%@] %@ - %lu: %@", category, fileName, (unsigned long)line, message);
        }
    };
    
    return mapboxHandler;
}

@end
#endif
