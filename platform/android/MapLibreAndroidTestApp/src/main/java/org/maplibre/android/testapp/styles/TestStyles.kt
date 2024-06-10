package org.maplibre.android.testapp.styles

import org.maplibre.android.maps.Style

object TestStyles {
    val VERSATILES = "https://tiles.versatiles.org/assets/styles/colorful.json"

    val AMERICANA = "https://zelonewolf.github.io/openstreetmap-americana/style.json"

    fun getPredefinedStyleWithFallback(name: String): String {
        try {
            val style = Style.getPredefinedStyle(name)
            return style
        } catch (e: Exception) {
            return VERSATILES
        }
    }
}