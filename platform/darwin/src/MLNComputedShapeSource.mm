#import "MLNComputedShapeSource_Private.h"

#import "MLNGeometry_Private.h"
#import "MLNMapView_Private.h"
#import "MLNShapeCollection.h"
#import "MLNShape_Private.h"
#import "MLNSource_Private.h"

#include <mln/map/map.hpp>
#include <mln/style/sources/custom_geometry_source.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/util/geojson.hpp>

const MLNExceptionName MLNInvalidDatasourceException = @"MLNInvalidDatasourceException";

const MLNShapeSourceOption MLNShapeSourceOptionWrapsCoordinates =
    @"MLNShapeSourceOptionWrapsCoordinates";
const MLNShapeSourceOption MLNShapeSourceOptionClipsCoordinates =
    @"MLNShapeSourceOptionClipsCoordinates";

mln::style::CustomGeometrySource::Options MBGLCustomGeometrySourceOptionsFromDictionary(
    NSDictionary<MLNShapeSourceOption, id> *options) {
  mln::style::CustomGeometrySource::Options sourceOptions;

  if (NSNumber *value = options[MLNShapeSourceOptionMinimumZoomLevel]) {
    if (![value isKindOfClass:[NSNumber class]]) {
      [NSException raise:NSInvalidArgumentException
                  format:@"MLNShapeSourceOptionMinimumZoomLevel must be an NSNumber."];
    }
    sourceOptions.zoomRange.min = value.integerValue;
  }

  if (NSNumber *value = options[MLNShapeSourceOptionMaximumZoomLevel]) {
    if (![value isKindOfClass:[NSNumber class]]) {
      [NSException raise:NSInvalidArgumentException
                  format:@"MLNShapeSourceOptionMaximumZoomLevel must be an NSNumber."];
    }
    sourceOptions.zoomRange.max = value.integerValue;
  }

  if (NSNumber *value = options[MLNShapeSourceOptionBuffer]) {
    if (![value isKindOfClass:[NSNumber class]]) {
      [NSException raise:NSInvalidArgumentException
                  format:@"MLNShapeSourceOptionBuffer must be an NSNumber."];
    }
    sourceOptions.tileOptions.buffer = value.integerValue;
  }

  if (NSNumber *value = options[MLNShapeSourceOptionSimplificationTolerance]) {
    if (![value isKindOfClass:[NSNumber class]]) {
      [NSException raise:NSInvalidArgumentException
                  format:@"MLNShapeSourceOptionSimplificationTolerance must be an NSNumber."];
    }
    sourceOptions.tileOptions.tolerance = value.doubleValue;
  }

  if (NSNumber *value = options[MLNShapeSourceOptionWrapsCoordinates]) {
    if (![value isKindOfClass:[NSNumber class]]) {
      [NSException raise:NSInvalidArgumentException
                  format:@"MLNShapeSourceOptionWrapsCoordinates must be an NSNumber."];
    }
    sourceOptions.tileOptions.wrap = value.boolValue;
  }

  if (NSNumber *value = options[MLNShapeSourceOptionClipsCoordinates]) {
    if (![value isKindOfClass:[NSNumber class]]) {
      [NSException raise:NSInvalidArgumentException
                  format:@"MLNShapeSourceOptionClipsCoordinates must be an NSNumber."];
    }
    sourceOptions.tileOptions.clip = value.boolValue;
  }

  return sourceOptions;
}

@interface MLNComputedShapeSource () {
  std::unique_ptr<mln::style::CustomGeometrySource> _pendingSource;
}

@property (nonatomic, readwrite) NSDictionary *options;
@property (nonatomic, assign) BOOL dataSourceImplementsFeaturesForTile;
@property (nonatomic, assign) BOOL dataSourceImplementsFeaturesForBounds;

@end

@interface MLNComputedShapeSourceFetchOperation : NSOperation

@property (nonatomic, readonly) uint8_t z;
@property (nonatomic, readonly) uint32_t x;
@property (nonatomic, readonly) uint32_t y;
@property (nonatomic, assign) BOOL dataSourceImplementsFeaturesForTile;
@property (nonatomic, assign) BOOL dataSourceImplementsFeaturesForBounds;
@property (nonatomic, weak, nullable) id<MLNComputedShapeSourceDataSource> dataSource;
@property (nonatomic, nullable) mln::style::CustomGeometrySource *rawSource;

- (instancetype)initForSource:(MLNComputedShapeSource *)source
                         tile:(const mln::CanonicalTileID &)tileId;

@end

@implementation MLNComputedShapeSourceFetchOperation

- (instancetype)initForSource:(MLNComputedShapeSource *)source
                         tile:(const mln::CanonicalTileID &)tileID {
  self = [super init];
  _z = tileID.z;
  _x = tileID.x;
  _y = tileID.y;
  _dataSourceImplementsFeaturesForTile = source.dataSourceImplementsFeaturesForTile;
  _dataSourceImplementsFeaturesForBounds = source.dataSourceImplementsFeaturesForBounds;
  _dataSource = source.dataSource;
  mln::style::CustomGeometrySource *rawSource =
      static_cast<mln::style::CustomGeometrySource *>(source.rawSource);
  _rawSource = rawSource;
  return self;
}

- (void)main {
  if ([self isCancelled]) {
    return;
  }

  NSArray<MLNShape<MLNFeature> *> *data;
  if (!self.dataSource) {
    data = nil;
  } else if (self.dataSourceImplementsFeaturesForTile) {
    data = [self.dataSource featuresInTileAtX:self.x y:self.y zoomLevel:self.z];
  } else {
    mln::CanonicalTileID tileID = mln::CanonicalTileID(self.z, self.x, self.y);
    mln::LatLngBounds tileBounds = mln::LatLngBounds(tileID);
    data =
        [self.dataSource featuresInCoordinateBounds:MLNCoordinateBoundsFromLatLngBounds(tileBounds)
                                          zoomLevel:self.z];
  }

  if (![self isCancelled]) {
    mln::FeatureCollection featureCollection;
    featureCollection.reserve(data.count);
    for (MLNShape<MLNFeature> *feature in data) {
      if ([feature isMemberOfClass:[MLNShapeCollection class]]) {
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
          NSLog(@"MLNShapeCollection initialized with MLNFeatures will not retain attributes."
                @"Use MLNShapeCollectionFeature to retain attributes instead."
                @"This will be logged only once.");
        });
      }
      mln::GeoJSONFeature geoJsonObject = [feature geoJSONObject].get<mln::GeoJSONFeature>();
      featureCollection.push_back(geoJsonObject);
    }
    const auto geojson = mln::GeoJSON{featureCollection};

    // Note: potential race condition with `cancel`
    if (![self isCancelled]) {
      if (auto *rawSource = self.rawSource) {
        rawSource->setTileData(mln::CanonicalTileID(self.z, self.x, self.y), geojson);
      }
    }
  }
}

- (void)cancel {
  [super cancel];
  self.rawSource = NULL;
}

@end

@implementation MLNComputedShapeSource

- (instancetype)initWithIdentifier:(NSString *)identifier
                           options:(NSDictionary<MLNShapeSourceOption, id> *)options {
  NSOperationQueue *requestQueue = [[NSOperationQueue alloc] init];
  requestQueue.name = [NSString stringWithFormat:@"mgl.MLNComputedShapeSource.%@", identifier];
  requestQueue.qualityOfService = NSQualityOfServiceUtility;
  requestQueue.maxConcurrentOperationCount = 4;

  auto sourceOptions = MBGLCustomGeometrySourceOptionsFromDictionary(options);
  sourceOptions.fetchTileFunction = ^void(const mln::CanonicalTileID &tileID) {
    NSOperation *operation = [[MLNComputedShapeSourceFetchOperation alloc] initForSource:self
                                                                                    tile:tileID];
    [requestQueue addOperation:operation];
  };

  sourceOptions.cancelTileFunction = ^void(const mln::CanonicalTileID &tileID) {
    for (MLNComputedShapeSourceFetchOperation *operation in requestQueue.operations) {
      if (operation.x == tileID.x && operation.y == tileID.y && operation.z == tileID.z) {
        [operation cancel];
      }
    }
  };

  auto source =
      std::make_unique<mln::style::CustomGeometrySource>(identifier.UTF8String, sourceOptions);

  if (self = [super initWithPendingSource:std::move(source)]) {
    _requestQueue = requestQueue;
  }
  return self;
}

- (instancetype)initWithIdentifier:(NSString *)identifier
                        dataSource:(id<MLNComputedShapeSourceDataSource>)dataSource
                           options:(NSDictionary<MLNShapeSourceOption, id> *)options {
  if (self = [self initWithIdentifier:identifier options:options]) {
    [self setDataSource:dataSource];
  }
  return self;
}

- (void)dealloc {
  [self.requestQueue cancelAllOperations];
}

- (void)setFeatures:(NSArray<MLNShape<MLNFeature> *> *)features
          inTileAtX:(NSUInteger)x
                  y:(NSUInteger)y
          zoomLevel:(NSUInteger)zoomLevel {
  MLNAssertStyleSourceIsValid();
  mln::CanonicalTileID tileID = mln::CanonicalTileID((uint8_t)zoomLevel, (uint32_t)x, (uint32_t)y);
  mln::FeatureCollection featureCollection;
  featureCollection.reserve(features.count);
  for (MLNShape<MLNFeature> *feature in features) {
    mln::GeoJSONFeature geoJsonObject = [feature geoJSONObject].get<mln::GeoJSONFeature>();
    featureCollection.push_back(geoJsonObject);
    if ([feature isMemberOfClass:[MLNShapeCollection class]]) {
      static dispatch_once_t onceToken;
      dispatch_once(&onceToken, ^{
        NSLog(@"MLNShapeCollection initialized with MLNFeatures will not retain attributes."
              @"Use MLNShapeCollectionFeature to retain attributes instead."
              @"This will be logged only once.");
      });
    }
  }
  const auto geojson = mln::GeoJSON{featureCollection};
  static_cast<mln::style::CustomGeometrySource *>(self.rawSource)->setTileData(tileID, geojson);
}

- (void)setDataSource:(id<MLNComputedShapeSourceDataSource>)dataSource {
  [self.requestQueue cancelAllOperations];
  // Check which method the datasource implements, to avoid having to check for each tile
  self.dataSourceImplementsFeaturesForTile =
      [dataSource respondsToSelector:@selector(featuresInTileAtX:y:zoomLevel:)];
  self.dataSourceImplementsFeaturesForBounds =
      [dataSource respondsToSelector:@selector(featuresInCoordinateBounds:zoomLevel:)];

  if (!self.dataSourceImplementsFeaturesForBounds && !self.dataSourceImplementsFeaturesForTile) {
    [NSException
         raise:MLNInvalidDatasourceException
        format:@"Datasource does not implement any MLNComputedShapeSourceDataSource methods"];
  } else if (self.dataSourceImplementsFeaturesForBounds &&
             self.dataSourceImplementsFeaturesForTile) {
    [NSException raise:MLNInvalidDatasourceException
                format:@"Datasource implements multiple MLNComputedShapeSourceDataSource methods"];
  }

  _dataSource = dataSource;
}

- (void)invalidateBounds:(MLNCoordinateBounds)bounds {
  MLNAssertStyleSourceIsValid();
  ((mln::style::CustomGeometrySource *)self.rawSource)
      ->invalidateRegion(MLNLatLngBoundsFromCoordinateBounds(bounds));
}

- (void)invalidateTileAtX:(NSUInteger)x y:(NSUInteger)y zoomLevel:(NSUInteger)z {
  MLNAssertStyleSourceIsValid();
  ((mln::style::CustomGeometrySource *)self.rawSource)
      ->invalidateTile(mln::CanonicalTileID(z, (unsigned int)x, (unsigned int)y));
}

@end
