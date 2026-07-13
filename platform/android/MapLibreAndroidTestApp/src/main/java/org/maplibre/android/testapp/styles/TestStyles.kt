package org.maplibre.android.testapp.styles

import org.maplibre.android.maps.Style

object TestStyles {
    const val DEMOTILES = "https://demotiles.maplibre.org/style.json"

    const val AMERICANA = "https://americanamap.org/style.json"

    const val OPENFREEMAP_LIBERTY = "https://tiles.openfreemap.org/styles/liberty"

    const val OPENFREEMAP_BRIGHT = "https://tiles.openfreemap.org/styles/bright"

    const val AWS_OPEN_DATA_STANDARD_LIGHT = "https://maps.geo.us-east-2.amazonaws.com/maps/v0/maps/OpenDataStyle/style-descriptor?key=v1.public.eyJqdGkiOiI1NjY5ZTU4My0yNWQwLTQ5MjctODhkMS03OGUxOTY4Y2RhMzgifR_7GLT66TNRXhZJ4KyJ-GK1TPYD9DaWuc5o6YyVmlikVwMaLvEs_iqkCIydspe_vjmgUVsIQstkGoInXV_nd5CcmqRMMa-_wb66SxDdbeRDvmmkpy2Ow_LX9GJDgL2bbiCws0wupJPFDwWCWFLwpK9ICmzGvNcrPbX5uczOQL0N8V9iUvziA52a1WWkZucIf6MUViFRf3XoFkyAT15Ll0NDypAzY63Bnj8_zS8bOaCvJaQqcXM9lrbTusy8Ftq8cEbbK5aMFapXRjug7qcrzUiQ5sr0g23qdMvnKJQFfo7JuQn8vwAksxrQm6A0ByceEXSfyaBoVpFcTzEclxUomhY.NjAyMWJkZWUtMGMyOS00NmRkLThjZTMtODEyOTkzZTUyMTBi"

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
