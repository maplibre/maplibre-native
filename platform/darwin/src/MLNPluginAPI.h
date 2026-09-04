#pragma once

/**
 The stable, C-only interface used to register MapLibre Native plugins.

 Plugin packages should include this umbrella header instead of depending on
 MapLibre Native C++ headers. Backend objects passed through callbacks are
 borrowed for the duration of that callback.
 */
#if __has_include(<mln/plugin/plugin_api.h>)
#include <mln/plugin/plugin_api.h>
#else
#include "plugin_api.h"
#endif
