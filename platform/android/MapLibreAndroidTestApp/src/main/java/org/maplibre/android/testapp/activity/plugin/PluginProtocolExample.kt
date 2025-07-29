package org.maplibre.android.testapp.activity.plugin

import org.maplibre.android.plugin.PluginProtocolHandler
import org.maplibre.android.plugin.PluginProtocolHandlerResource
import org.maplibre.android.plugin.PluginProtocolHandlerResponse
import java.net.URI


class PluginProtocolExample : PluginProtocolHandler() {

    override fun canRequestResource(resource: PluginProtocolHandlerResource?): Boolean {

        return true;

    }


    override fun requestResource(resource: PluginProtocolHandlerResource?): PluginProtocolHandlerResponse? {
        // Return some data here
        var tempResult = PluginProtocolHandlerResponse();

        val tempStyle: String  =
                "{\n" +
                        "    \"id\": \"43f36e14-e3f5-43c1-84c0-50a9c80dc5c7\",\n" +
                        "    \"name\": \"MapLibre\",\n" +
                        "    \"zoom\": 0.8619833357855968,\n" +
                        "    \"pitch\": 0,\n" +
                        "    \"center\": [\n" +
                        "        17.65431710431244,\n" +
                        "        32.954120326746775\n" +
                        "    ],\n" +
                        "    \"layers\": [\n" +
                        "        {\n" +
                        "            \"id\": \"background\",\n" +
                        "            \"type\": \"background\",\n" +
                        "            \"paint\": {\n" +
                        "                \"background-color\": \"#000000\"\n" +
                        "            },\n" +
                        "            \"filter\": [\n" +
                        "                \"all\"\n" +
                        "            ],\n" +
                        "            \"layout\": {\n" +
                        "                \"visibility\": \"visible\"\n" +
                        "            },\n" +
                        "            \"maxzoom\": 24\n" +
                        "        }\n"
                        "       ]\n"
                       " } \n"

        tempResult.generateBuffer(tempStyle);
        return tempResult;
    }

}
