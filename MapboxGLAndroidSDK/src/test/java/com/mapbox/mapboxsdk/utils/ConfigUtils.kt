package com.mapbox.mapboxsdk.utils

import com.mapbox.mapboxsdk.util.DefaultStyle
import com.mapbox.mapboxsdk.util.TileServerOptions

class ConfigUtils {
    companion object {
        @JvmStatic
        fun getMockedOptions(): TileServerOptions {
            val defaultStyles = arrayOf(
                DefaultStyle("maptiler://maps/streets", "Streets", 1),
                DefaultStyle("maptiler://maps/outdoor", "Outdoor", 1),
                DefaultStyle("maptiler://maps/basic", "Basic", 1),
                DefaultStyle("maptiler://maps/bright", "Bright", 1),
                DefaultStyle("maptiler://maps/pastel", "Pastel", 1),
                DefaultStyle("maptiler://maps/hybrid", "Satellite Hybrid", 1),
                DefaultStyle("maptiler://maps/topo", "Satellite Topo", 1)
            )
            val defaultStyle = defaultStyles[0].name

            return TileServerOptions(
                "https://api.maptiler.com",
                "maptiler",
                "{path}",
                "sources",
                null,
                "/maps{path}/style.json",
                "maps",
                null,
                "/maps{path}",
                "sprites",
                null,
                "/fonts{path}",
                "fonts",
                null,
                "{path}",
                "tiles",
                null,
                "key",
                true,
                defaultStyle,
                defaultStyles
            )
        }
    }
}
