#import "iosTestRunner.h"

#include <ios_test_runner.hpp>

#include <string>

@interface IosTestRunner ()

@property (nullable) TestRunner *runner;

@property (copy, nullable) NSString *resultPath;

@property BOOL testStatus;

@end

@implementation IosTestRunner

-(instancetype)init
{
    self = [super init];
    if (self) {
        self.testStatus = NO;
        self.runner = new TestRunner();
        BOOL success;

        NSError *error;
        NSFileManager *fileManager = [NSFileManager defaultManager];

        NSString *xcTestBundleRoot = [[NSBundle bundleForClass:[self class]] resourcePath];
        NSString *testDataDir = [xcTestBundleRoot stringByAppendingString:@"/TestData.bundle"];

        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documentsDir = [paths objectAtIndex: 0];

        NSString *destinationPath = [documentsDir stringByAppendingPathComponent: @"test"];

        success = [fileManager copyItemAtPath: testDataDir toPath: destinationPath error: &error];
        if (!success){
            NSAssert1(0, @"Failed to copy file '%@'.", [error localizedDescription]);
            NSLog(@"Failed to copy %@ file, error %@", testDataDir, [error localizedDescription]);
        }
        else {
            NSLog(@"File copied %@ OK", testDataDir);
        }

        NSLog(@"Starting test");
        std::string basePath = std::string([documentsDir UTF8String]);
        self.testStatus = self.runner->startTest(basePath) ? YES : NO;
        self.resultPath = [destinationPath stringByAppendingPathComponent: @"results.xml"];

        BOOL fileFound = [fileManager fileExistsAtPath: self.resultPath];
        if (fileFound == NO) {
            NSLog(@"Test result file '%@' does not exist", self.resultPath);
            self.testStatus = NO;
        }

        delete self.runner;
        self.runner = nullptr;
    }
    return self;
}

- (NSString*) getResultPath {
    return self.resultPath;
}

- (BOOL) getTestStatus {
    return self.testStatus;
}
@end
