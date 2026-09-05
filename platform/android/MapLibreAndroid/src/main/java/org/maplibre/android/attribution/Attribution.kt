package org.maplibre.android.attribution

class Attribution internal constructor(
    val title: String,
    val url: String,
) {
    val titleAbbreviated: String
        get() = if (title == OPENSTREETMAP) OPENSTREETMAP_ABBR else title

    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (other == null || javaClass != other.javaClass) {
            return false
        }

        other as Attribution

        return title == other.title && url == other.url
    }

    override fun hashCode(): Int {
        var result = title.hashCode()
        result = 31 * result + url.hashCode()
        return result
    }

    companion object {
        private const val OPENSTREETMAP = "OpenStreetMap"
        private const val OPENSTREETMAP_ABBR = "OSM"
        internal const val MAPBOX_URL = "https://www.mapbox.com/about/maps/"

        // Using a List makes URL backwards compatible
        internal val IMPROVE_MAP_URLS: List<String> =
            listOf(
                "https://www.mapbox.com/feedback/",
                "https://www.mapbox.com/map-feedback/",
                "https://apps.mapbox.com/feedback/",
            )
    }
}
