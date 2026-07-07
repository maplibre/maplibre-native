#import "StyleFilterExample.h"

@implementation StyleFilterExample

- (NSString *)filterName {
  return @"layer-filter";
}

- (NSData *)filterData:(NSData *)data {
  NSError *error = nil;
  NSMutableDictionary *styleDictionary =
      [NSJSONSerialization JSONObjectWithData:data
                                      options:NSJSONReadingMutableContainers
                                        error:&error];

  NSData *tempResult = data;
  if (styleDictionary) {
    NSMutableArray *layerArray = [styleDictionary objectForKey:@"layers"];

    NSMutableArray *removedLayers = [NSMutableArray array];

    for (NSMutableDictionary *layer in layerArray) {
      NSString *layerID = [layer objectForKey:@"id"];

      if ([layerID containsString:@"metal-rendering-layer"]) {
        [removedLayers addObject:layer];
      }
    }

    for (NSMutableDictionary *l in removedLayers) {
      [layerArray removeObject:l];
    }

    NSData *filteredStyleJSON = [NSJSONSerialization dataWithJSONObject:styleDictionary
                                                                options:0
                                                                  error:&error];

    if (filteredStyleJSON) {
      tempResult = filteredStyleJSON;
    }
  }

  return tempResult;
}

@end
