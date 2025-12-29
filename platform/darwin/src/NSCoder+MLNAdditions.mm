#import "NSCoder+MLNAdditions.h"

#import "NSArray+MLNAdditions.h"
#import "NSValue+MLNAdditions.h"

@implementation NSCoder (MLNAdditions)

- (void)mgl_encodeLocationCoordinates2D:(std::vector<CLLocationCoordinate2D>)coordinates forKey:(NSString *)key {
    [self encodeObject:[NSArray mgl_coordinatesFromCoordinates:coordinates] forKey:key];
}

- (std::vector<CLLocationCoordinate2D>)mgl_decodeLocationCoordinates2DForKey:(NSString *)key {
    NSSet<Class> *coordinateClasses = [NSSet setWithArray:@[[NSArray class], [NSDictionary class]]];
    NSArray *coordinates = [self decodeObjectOfClasses:coordinateClasses forKey:key];
    return [coordinates mgl_coordinates];
}

- (void)encodeMLNCoordinate:(CLLocationCoordinate2D)coordinate forKey:(NSString *)key {
    [self encodeObject:@{@"latitude": @(coordinate.latitude), @"longitude": @(coordinate.longitude)} forKey:key];
}

- (CLLocationCoordinate2D)decodeMLNCoordinateForKey:(NSString *)key {
    NSDictionary *coordinate = [self decodeObjectOfClass:[NSDictionary class] forKey:key];
    return CLLocationCoordinate2DMake([coordinate[@"latitude"] doubleValue], [coordinate[@"longitude"] doubleValue]);
}

@end
