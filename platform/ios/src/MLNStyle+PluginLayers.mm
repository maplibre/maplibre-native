#import "MLNStyle+PluginLayers.h"

#include <mln/style/conversion/layer.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/style.hpp>

#import "MLNConversion.h"
#import "MLNStyle_Private.h"

#include <memory>
#include <optional>
#include <stdexcept>

static NSString *const MLNPluginLayersErrorDomain = @"MLNPluginLayersErrorDomain";

static BOOL MLNFail(NSError **error, NSString *message) {
  if (error) {
    *error = [NSError errorWithDomain:MLNPluginLayersErrorDomain
                                 code:1
                             userInfo:@{NSLocalizedDescriptionKey : message}];
  }
  return NO;
}

@implementation MLNStyle (PluginLayers)

- (BOOL)addLayerWithJSON:(NSDictionary<NSString *, id> *)layerJSON
    belowLayerWithIdentifier:(NSString *)siblingIdentifier
                       error:(NSError **)error {
  mln::style::conversion::Error conversionError;
  auto layer = mln::style::conversion::convert<std::unique_ptr<mln::style::Layer>>(
      mln::style::conversion::makeConvertible(layerJSON), conversionError);
  if (!layer) {
    return MLNFail(error, @(conversionError.message.c_str()));
  }

  std::optional<std::string> sibling;
  if (siblingIdentifier) {
    sibling = std::string(siblingIdentifier.UTF8String);
    if (!self.rawStyle->getLayer(*sibling)) {
      return MLNFail(error, [NSString stringWithFormat:@"No layer named %@ to insert below", siblingIdentifier]);
    }
  }

  try {
    self.rawStyle->addLayer(std::move(*layer), sibling);
  } catch (const std::runtime_error &exception) {
    return MLNFail(error, @(exception.what()));
  }
  return YES;
}

- (BOOL)setProperty:(NSString *)name
                   value:(id)value
    forLayerWithIdentifier:(NSString *)layerIdentifier
                     error:(NSError **)error {
  auto *layer = self.rawStyle->getLayer(layerIdentifier.UTF8String);
  if (!layer) {
    return MLNFail(error, [NSString stringWithFormat:@"No layer named %@", layerIdentifier]);
  }

  auto failure = layer->setProperty(name.UTF8String, mln::style::conversion::makeConvertible(value));
  if (failure) {
    return MLNFail(error, @(failure->message.c_str()));
  }
  return YES;
}

@end
