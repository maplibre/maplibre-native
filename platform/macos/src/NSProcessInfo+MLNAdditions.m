#import "NSProcessInfo+MLNAdditions.h"

@implementation NSProcessInfo (MLNAdditions)

- (BOOL)mgl_isInterfaceBuilderDesignablesAgent {
    NSString *processName = self.processName;
    return [processName hasPrefix:@"IBAgent"] || [processName hasPrefix:@"IBDesignablesAgent"];
}

@end
