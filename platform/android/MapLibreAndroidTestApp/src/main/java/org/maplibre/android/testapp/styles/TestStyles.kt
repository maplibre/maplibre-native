package org.maplibre.android.testapp.styles

import org.maplibre.android.maps.Style

object TestStyles {
    const val DEMOTILES = "https://demotiles.maplibre.org/style.json"

    const val AMERICANA = "https://americanamap.org/style.json"

    const val OPENFREEMAP_LIBERTY = "https://tiles.openfreemap.org/styles/liberty"

    const val OPENFREEMAP_BRIGHT = "https://tiles.openfreemap.org/styles/bright"

    private fun protomaps(style: String): String {
        return "https://api.protomaps.com/styles/v2/${style}.json?key=e761cc7daedf832a"
    }
    val PROTOMAPS_LIGHT = protomaps("light")

    val PROTOMAPS_DARK = protomaps("dark")

    val PROTOMAPS_GRAYSCALE = protomaps("grayscale")

    val PROTOMAPS_WHITE = protomaps("white")

    val PROTOMAPS_BLACK = protomaps("black")

    fun getPredefinedStyleWithFallback(name: String): String {
        try {
            val style = Style.getPredefinedStyle(name)
            return style
        } catch (e: Exception) {
            return OPENFREEMAP_LIBERTY
        }
    }
}
