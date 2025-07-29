package org.maplibre.android.plugin;

public class PluginFileSource {

    public PluginProtocolHandler protocolHandler;

    public boolean canRequestResource(PluginProtocolHandlerResource resource) {

        if (protocolHandler.canRequestResource(resource)) {
            return true;
        }
        return false;

    }
/*
    public boolean canRequestResource() {
        if (protocolHandler.canRequestResource(resource)) {
            return true;
        }
        return false;
    }
*/
    public PluginProtocolHandlerResponse requestResource(PluginProtocolHandlerResource resource) {
        PluginProtocolHandlerResponse response = protocolHandler.requestResource(resource);
        return response;
    }

}
