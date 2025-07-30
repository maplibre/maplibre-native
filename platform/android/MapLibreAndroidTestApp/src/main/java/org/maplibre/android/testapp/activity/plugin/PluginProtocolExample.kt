package org.maplibre.android.testapp.activity.plugin

import org.maplibre.android.plugin.PluginProtocolHandler
import org.maplibre.android.plugin.PluginProtocolHandlerResource
import org.maplibre.android.plugin.PluginProtocolHandlerResponse
import java.net.URI


class PluginProtocolExample : PluginProtocolHandler() {
    var styleLoaded: Boolean = false;

    override fun canRequestResource(resource: PluginProtocolHandlerResource?): Boolean {
        if (!styleLoaded) {
            styleLoaded = true;
            return true;
        }
        return false;

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
                        "        },\n"+
                "        {\n" +
                "            \"id\": \"coastline\",\n" +
                "            \"type\": \"line\",\n" +
                "            \"paint\": {\n" +
                "                \"line-blur\": 0.5,\n" +
                "                \"line-color\": \"#198EC8\",\n" +
                "                \"line-width\": {\n" +
                "                    \"stops\": [\n" +
                "                        [\n" +
                "                            0,\n" +
                "                            2\n" +
                "                        ],\n" +
                "                        [\n" +
                "                            6,\n" +
                "                            6\n" +
                "                        ],\n" +
                "                        [\n" +
                "                            14,\n" +
                "                            9\n" +
                "                        ],\n" +
                "                        [\n" +
                "                            22,\n" +
                "                            18\n" +
                "                        ]\n" +
                "                    ]\n" +
                "                }\n" +
                "            },\n" +
                "            \"filter\": [\n" +
                "                \"all\"\n" +
                "            ],\n" +
                "            \"layout\": {\n" +
                "                \"line-cap\": \"round\",\n" +
                "                \"line-join\": \"round\",\n" +
                "                \"visibility\": \"visible\"\n" +
                "            },\n" +
                "            \"source\": \"maplibre\",\n" +
                "            \"maxzoom\": 24,\n" +
                "            \"minzoom\": 0,\n" +
                "            \"source-layer\": \"countries\"\n" +
                "        }"+
                        "       ],\n"+
        " \"sources\": {\n" +
                "        \"maplibre\": {\n" +
                "            \"url\": \"https://demotiles.maplibre.org/tiles/tiles.json\",\n" +
                "            \"type\": \"vector\"\n" +
                "        },"+
        "    \"version\": 8\n"+
                       " } }\n";

        tempResult.generateBuffer(tempStyle);
        return tempResult;
    }

}
