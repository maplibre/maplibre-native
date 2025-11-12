package org.maplibre.android.plugin;

public class PluginFileSource {

  public PluginProtocolHandler protocolHandler;

  public boolean canRequestResource(PluginProtocolHandlerResource resource) {

    return protocolHandler.canRequestResource(resource);

  }

  public PluginProtocolHandlerResponse requestResource(PluginProtocolHandlerResource resource) {

    PluginProtocolHandlerResponse response = protocolHandler.requestResource(resource);
    return response;

  }

}
