package org.maplibre.android.testapp.activity.plugin

import org.maplibre.android.plugin.PluginProtocolHandler
import org.maplibre.android.plugin.PluginProtocolHandlerResource
import org.maplibre.android.plugin.PluginProtocolHandlerResponse

class PluginProtocolExample : PluginProtocolHandler() {

    override fun canRequestResource(resource: PluginProtocolHandlerResource?): Boolean {

        if (resource == null) {
            return false;
        }

        if (resource.resourceURL != null) {
            if (resource.resourceURL!!.contains("pluginProtocol")) {
                return true;
            }
        }

        if (resource.resourceKind == PluginProtocolHandlerResource.PluginProtocolHandlerResourceKind.Unknown) {
            return true;
        }

        return false;
    }


    override fun requestResource(resource: PluginProtocolHandlerResource?): PluginProtocolHandlerResponse? {
        // Return some data here
        var tempResult = PluginProtocolHandlerResponse();
        tempResult.generateBuffer();
        return tempResult;
    }

}