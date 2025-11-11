#import "MLNStyleLayer.h"

NS_ASSUME_NONNULL_BEGIN

@class MLNPluginLayer;

MLN_EXPORT
@interface MLNPluginStyleLayer : MLNStyleLayer

- (MLNPluginLayer *)pluginLayer;

- (instancetype)initWithType:(NSString *)layerType
             layerIdentifier:(NSString *)identifier
              layerPropeties:(NSDictionary *)layerPropeties;

- (void)setPluginProperty:(NSString *)propertyName value:(id)propertyValue;

@end

NS_ASSUME_NONNULL_END
