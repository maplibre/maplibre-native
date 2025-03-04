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

        NSFileManager *fileManager = [NSFileManager defaultManager];

        NSString *xcTestBundleRoot = [[NSBundle bundleForClass:[self class]] resourcePath];
        NSString *testDataDir = [xcTestBundleRoot stringByAppendingString:@"/TestData.bundle"];

        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documentsDir = [paths objectAtIndex: 0];

        NSString *destinationPath = [documentsDir stringByAppendingPathComponent: @"test"];

        [self copyFileAtPath:testDataDir toPath:destinationPath];
        [self copyFileAtPath:[testDataDir stringByAppendingPathComponent: @"scripts"] toPath:[documentsDir stringByAppendingPathComponent: @"scripts"]];

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

/**
 Copies a file from the source path to the destination path.

 @param sourcePath The path of the source file to be copied.
 @param destinationPath The destination path where the file should be copied.
 */
- (void)copyFileAtPath:(NSString *)sourcePath toPath:(NSString *)destinationPath {
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSError *error = nil;
    BOOL success;

    // delete destinationPath if it exists
    if ([fileManager fileExistsAtPath:destinationPath]) {
        success = [fileManager removeItemAtPath: destinationPath error: &error];
        if (!success) {
            NSAssert1(0, @"Failed to delete '%@'.", destinationPath);
        }
    }

    success = [fileManager copyItemAtPath:sourcePath toPath:destinationPath error:&error];
    if (!success) {
        NSAssert1(0, @"Failed to copy file '%@'.", [error localizedDescription]);
        NSLog(@"Failed to copy %@ file, error %@", sourcePath, [error localizedDescription]);
    } else {
        NSLog(@"File copied %@ OK", sourcePath);
    }
}

- (NSString*) getResultPath {
    return self.resultPath;
}

- (BOOL) getTestStatus {
    return self.testStatus;
}
@end
