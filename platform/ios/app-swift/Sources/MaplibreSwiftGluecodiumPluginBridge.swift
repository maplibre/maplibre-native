// Swift bridge for Gluecodium MaplibrePlugin classes
// This creates a MapLibre plugin bridge from a Gluecodium plugin

import MapLibre
import ArrowPolylineGluecodiumPluginBridge
import GluecodiumArrowPolyline

/// Swift bridge class that wraps a Gluecodium plugin for use with MLNMapView.
/// This class holds a strong reference to the plugin to keep it alive.
public class MaplibreSwiftGluecodiumPluginBridge: MaplibreGluecodiumPluginBridge {
  private let plugin: MaplibrePlugin

  public init(plugin: MaplibrePlugin) {
    self.plugin = plugin
    super.init(pluginPtr: plugin.ptr)
  }
}

/// Extension on Gluecodium's MaplibrePlugin to create a bridge for MLNMapView
public extension MaplibrePlugin {
  /// Create a bridge that can be passed to MLNMapView's plugins parameter
  var bridge: MaplibreSwiftGluecodiumPluginBridge {
    return MaplibreSwiftGluecodiumPluginBridge(plugin: self)
  }
}
