package org.maplibre.android.plugin

import android.icu.text.Normalizer

public open class PluginProtocolHandler {

    open fun canRequestResource(resource: PluginProtocolHandlerResource?): Boolean {
        // Base class does nothing
        return false;
    }


    open fun requestResource(resource: PluginProtocolHandlerResource?): PluginProtocolHandlerResponse? {
        // Base class does nothing
        return null;
    }

}
