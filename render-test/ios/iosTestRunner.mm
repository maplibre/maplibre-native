#import "iosTestRunner.h"

#include "ios_test_runner.hpp"

#import "SSZipArchive.h"

#include <string>

@interface IosTestRunner ()

@property (nullable) TestRunner* runner;

@property (copy, nullable) NSString *styleResultPath;

@property (copy, nullable) NSString *metricResultPath;

@property (copy, nullable) NSString *metricPath;

@property BOOL testStatus;

@end

@implementation IosTestRunner

-(instancetype)init
{
    self = [super init];
    if (!self) {
        return nil;
    }
    
    self.testStatus = NO;
    self.runner = new TestRunner();
    NSError *error;
    BOOL success;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *xcTestBundleRoot = [[NSBundle bundleForClass:[self class]] resourcePath];
    NSString *testDataDir = [xcTestBundleRoot stringByAppendingString:@"/TestData.bundle"];

    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDir = [paths objectAtIndex: 0];
    
    NSString *destinationPath = [documentsDir stringByAppendingPathComponent: @"test-data"];
    NSString *path = destinationPath;
    
    // delete destinationPath if it exists
    if ([fileManager fileExistsAtPath:destinationPath]) {
        success = [fileManager removeItemAtPath: destinationPath error: &error];
        if (!success) {
            NSAssert1(0, @"Failed to delete '%@'.", destinationPath);
        }
    }

    success = [fileManager copyItemAtPath: testDataDir toPath: destinationPath error: &error];
    if (!success){
        NSAssert1(0, @"Failed to copy file '%@'.", [error localizedDescription]);
        NSLog(@"Failed to copy %@ file, error %@", testDataDir, [error localizedDescription]);
    }
    else {
        path = destinationPath;
        NSLog(@"File copied %@ OK", testDataDir);
    }

    if (path) {
        std::string basePath = std::string([path UTF8String]);
        self.testStatus = self.runner->startTest(basePath) ? YES : NO;
        self.styleResultPath =  [path stringByAppendingPathComponent:@"/ios-render-test-runner-style.html"];
        self.metricResultPath =  [path stringByAppendingPathComponent:@"/ios-render-test-runner-metrics.html"];

        BOOL fileFound = [fileManager fileExistsAtPath: self.styleResultPath];
        if (fileFound == NO) {
            NSLog(@"Style test result file '%@' does not exit ", self.styleResultPath);
            self.testStatus = NO;
        }

        fileFound = [fileManager fileExistsAtPath: self.metricResultPath];
        if (fileFound == NO) {
            NSLog(@"Metric test result file '%@' does not exit ", self.metricResultPath);
            self.testStatus = NO;
        }

        NSString *rebaselinePath = [path stringByAppendingPathComponent:@"/baselines"];
        BOOL needArchiving = NO;
        BOOL isDir = NO;
        NSArray *subpaths = [[NSArray alloc] init];
        if ([fileManager fileExistsAtPath:rebaselinePath isDirectory: &isDir] && isDir){
            subpaths = [fileManager subpathsAtPath:rebaselinePath];
            for(NSString *path in subpaths)
            {
                NSString *longPath = [rebaselinePath stringByAppendingPathComponent:path];
                if([fileManager fileExistsAtPath:longPath isDirectory:&isDir] && !isDir)
                {
                    needArchiving = YES;
                    break;
                }
            }
        }
        else {
           NSLog(@"Metric path '%@' does not exit ", rebaselinePath);
        }

        if (needArchiving) {
            NSString *archivePath = [path stringByAppendingString:@"/metrics.zip"];
            BOOL success = [SSZipArchive createZipFileAtPath:archivePath
            withContentsOfDirectory:rebaselinePath
                keepParentDirectory:NO
                   compressionLevel:-1
                           password:nil
                                AES:YES
                    progressHandler:nil];

            if (success) {
                NSLog(@"Successfully archived all of the metrics into metrics.zip");
                self.metricPath =  archivePath;
            }
            else {
                NSLog(@"Failed to archive rebaselined metrics into metrics.zip");
            }
        }
    }

    delete self.runner;
    self.runner = nullptr;
    return self;
}

- (NSString*) getStyleResultPath {
    return self.styleResultPath;
}

- (NSString*) getMetricResultPath {
    return self.metricResultPath;
}

- (NSString*) getMetricPath {
    return self.metricPath;
}

- (BOOL) getTestStatus {
    return self.testStatus;
}
@end
