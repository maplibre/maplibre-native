package com.mapbox.mapboxsdk.utils

import com.mapbox.mapboxsdk.util.DefaultStyle
import com.mapbox.mapboxsdk.util.TileServerOptions

class Configuration {
    companion object {
        fun getMockedOptions(): TileServerOptions {
            val defaultStyle = DefaultStyle("maptiler://maps/streets", "Streets", 1)
            return TileServerOptions(
                "https://api.maptiler.com",
                "maptiler",
                "/tiles{path}/tiles.json",
                null,
                "/maps{path}/style.json",
                "maps",
                null,
                "/maps/{path}/sprite{scale}.{format}",
                "",
                null,
                "/fonts{path}",
                "fonts",
                null,
                "{path}",
                "tiles",
                null,
                "key",
                defaultStyle.name,
                arrayOf(defaultStyle)
            )
        }
    }
}