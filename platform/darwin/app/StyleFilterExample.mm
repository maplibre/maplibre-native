#import "StyleFilterExample.h"

@implementation StyleFilterExample

// This will filter the data passed in
-(NSData *)filterData:(NSData *)data {
    // Don't call super

    // This example will remove any layer that has "metal-rendering-layer" in the id

    // Parse the JSON: Make the containers mutable
    NSError *error = nil;
    NSMutableDictionary *styleDictionary = [NSJSONSerialization JSONObjectWithData:data
                                                                          options:NSJSONReadingMutableContainers
                                                                            error:&error];

    NSData *tempResult = data;
    if (styleDictionary) {

        // Get the layer array
        NSMutableArray *layerArray = [styleDictionary objectForKey:@"layers"];

        // Create an array to hold which objects to remove since we can't remove them in the loop
        NSMutableArray *removedLayers = [NSMutableArray array];

        // Loop the layers and look for any layers that have the search string in them
        for (NSMutableDictionary *layer in layerArray) {
            NSString *layerID = [layer objectForKey:@"id"];

            // If we find the layers we're looking for, add them to the list of layers to remove
            if ([layerID containsString:@"metal-rendering-layer"]) {
                [removedLayers addObject:layer];
            }
        }

        // Go through and remove any layers that were found
        for (NSMutableDictionary *l in removedLayers) {
            [layerArray removeObject:l];
        }

        // Re-create the JSON, this time the layers we filtered out won't be there
        NSData *filteredStyleJSON = [NSJSONSerialization dataWithJSONObject:styleDictionary
                                                                    options:0
                                                                      error:&error];

        // If the JSON write is successful, then set the output to the new json style
        if (filteredStyleJSON) {
            tempResult = filteredStyleJSON;
        }

    }

    // Return the data
    return tempResult;

}



@end
