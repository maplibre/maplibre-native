#import "NgonPlugin.h"

#include <mln/plugin/plugin_api.h>
#include <ngon_layer.hpp>

@implementation NgonPlugin

+ (BOOL)registerWithError:(NSError **)error {
  char message[512] = {};
  // The plugin registers through the entry point of the MapLibre it is linked with.
  const mln_plugin_status status =
      mln_ngon_layer_register(mln_plugin_register_v1, message, sizeof(message));
  if (status == MLN_PLUGIN_STATUS_OK || status == MLN_PLUGIN_STATUS_ALREADY_REGISTERED) {
    return YES;
  }
  if (error) {
    *error = [NSError errorWithDomain:@"NgonPluginErrorDomain"
                                 code:status
                             userInfo:@{NSLocalizedDescriptionKey : @(message)}];
  }
  return NO;
}

@end
