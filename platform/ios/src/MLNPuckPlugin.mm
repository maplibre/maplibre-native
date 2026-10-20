#import "MLNPuckPlugin.h"

#if MLN_WITH_PLUGINS
#include <mln/plugin/plugin_api.h>
#include <gltf_layer.hpp>
#endif

static NSString *const MLNPuckPluginErrorDomain = @"MLNPuckPluginErrorDomain";

@implementation MLNPuckPlugin

+ (BOOL)registerWithError:(NSError **)error {
#if MLN_WITH_PLUGINS
  using RegisterFunction = mln_plugin_status (*)(mln_plugin_register_function_v1, char *, size_t);
  const RegisterFunction registerFunctions[] = {&mln_gltf_layer_register};
  for (RegisterFunction registerLayer : registerFunctions) {
    char message[512] = {};
    const mln_plugin_status status =
        registerLayer(mln_plugin_register_v1, message, sizeof(message));
    if (status == MLN_PLUGIN_STATUS_OK || status == MLN_PLUGIN_STATUS_ALREADY_REGISTERED) {
      continue;
    }
    if (error) {
      *error = [NSError
          errorWithDomain:MLNPuckPluginErrorDomain
                     code:status
                 userInfo:@{NSLocalizedDescriptionKey : [NSString stringWithUTF8String:message]}];
    }
    return NO;
  }
  return YES;
#else
  if (error) {
    *error = [NSError errorWithDomain:MLNPuckPluginErrorDomain
                                 code:-1
                             userInfo:@{NSLocalizedDescriptionKey : @"MapLibre was built without plugin support"}];
  }
  return NO;
#endif
}

@end
