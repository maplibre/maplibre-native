#import "BenchmarkAdvancedMetrics.h"
#import <mach/mach.h>

@implementation BenchmarkAdvancedMetrics {
    NSTimer* timer;
    unsigned int snapshotCount;
    BenchmarkSnapshot total;
}

- (BenchmarkSnapshot)avg {
    BenchmarkSnapshot avg;

    avg.cpu = total.cpu / snapshotCount;
    avg.memory = total.memory / snapshotCount;

    return avg;
}

- (void)start:(NSTimeInterval)collectInterval {
    if (timer) {
        [self stop];
    }

    [self reset];

    timer = [NSTimer scheduledTimerWithTimeInterval:collectInterval repeats:YES block:^(NSTimer * _Nonnull timer) {
        [self collect];
    }];
}

- (void)stop {
    if (timer) {
        [timer invalidate];
        timer = nil;
    }
}

- (void)collect {
    BenchmarkSnapshot snapshot{
        [BenchmarkAdvancedMetrics getCpuUsage],
        [BenchmarkAdvancedMetrics getMemoryUsage]
    };

    _min.min(snapshot);
    _max.max(snapshot);
    total.add(snapshot);

    ++snapshotCount;
}

- (void)reset {
    _min = {FLT_MAX, ULLONG_MAX};
    _max = {-FLT_MAX, 0ull};
    total = {0.0f, 0ull};

    snapshotCount = 0;
}

+ (unsigned long long)getMemoryUsage {
    mach_task_basic_info_data_t info;
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;

    kern_return_t result = task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count);
    return result == KERN_SUCCESS ? info.resident_size : 0ull;
}

+ (float)getCpuUsage {
    mach_port_t* threadList = NULL;
    mach_msg_type_number_t threadCount = 0;
    thread_basic_info_data_t threadInfo;

    kern_return_t result = task_threads(mach_task_self(), &threadList, &threadCount);
    if (result != KERN_SUCCESS) {
        return 0.0f;
    }

    float cpuUsage = 0.0f;

    for (unsigned int i = 0; i < threadCount; ++i) {
        thread_info_data_t threadInfoData;
        mach_msg_type_number_t threadInfoCount = THREAD_INFO_MAX;

        result = thread_info(threadList[i], THREAD_BASIC_INFO, threadInfoData, &threadInfoCount);
        if (result != KERN_SUCCESS) {
            continue;
        }

        thread_basic_info_t basicInfo = (thread_basic_info_t)threadInfoData;
        if (basicInfo->flags & TH_FLAGS_IDLE) {
            continue;
        }

        cpuUsage += ((float)basicInfo->cpu_usage / TH_USAGE_SCALE) * 100.0f;
    }

    return cpuUsage;
}

@end
