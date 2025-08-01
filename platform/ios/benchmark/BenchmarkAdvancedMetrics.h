#import <Foundation/Foundation.h>

struct BenchmarkSnapshot {
  float cpu = 0.0f;
  unsigned long long memory = 0;

  void min(BenchmarkSnapshot s) {
    if (cpu > s.cpu) {
      cpu = s.cpu;
    }

    if (memory > s.memory) {
      memory = s.memory;
    }
  }

  void max(BenchmarkSnapshot s) {
    if (cpu < s.cpu) {
      cpu = s.cpu;
    }

    if (memory < s.memory) {
      memory = s.memory;
    }
  }

  void add(BenchmarkSnapshot s) {
    cpu += s.cpu;
    memory += s.memory;
  }
};

@interface BenchmarkAdvancedMetrics : NSObject

@property (nonatomic, readonly) BenchmarkSnapshot min;
@property (nonatomic, readonly) BenchmarkSnapshot max;
@property (nonatomic, readonly) BenchmarkSnapshot avg;

- (void)start:(NSTimeInterval)collectInterval;
- (void)stop;

+ (float)getCpuUsage;
+ (unsigned long long)getMemoryUsage;

@end
