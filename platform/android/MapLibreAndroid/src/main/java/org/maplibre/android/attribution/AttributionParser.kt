package org.maplibre.android.attribution

import android.content.Context
import android.os.Build
import android.text.Html
import android.text.SpannableStringBuilder
import android.text.Spanned
import android.text.style.URLSpan
import java.lang.ref.WeakReference

/**
 * Responsible for parsing attribution data coming from Sources and MapSnapshot.
 *
 * Exposes multiple configuration options to manipulate data being parsed.
 * Use the Options object to build these configurations.
 */
class AttributionParser internal constructor(
    private val context: WeakReference<Context>,
    private val attributionData: String,
    private val withImproveMap: Boolean,
    private val withCopyrightSign: Boolean,
    private val withMapboxAttribution: Boolean,
) {
    private val attributionSet: MutableSet<Attribution> = LinkedHashSet()

    /**
     * Get parsed attributions.
     */
    val attributions: Set<Attribution>
        get() = attributionSet

    /**
     * Get parsed attribution string.
     *
     * @param shortenedOutput if attribution string should contain shortened output
     * @return the parsed attribution string
     */
    @JvmOverloads
    fun createAttributionString(shortenedOutput: Boolean = false): String {
        if (attributions.isEmpty()) {
            return ""
        }
        val stringBuilder = StringBuilder(if (withCopyrightSign) "" else "© ")
        var counter = 0
        for (attribution in attributions) {
            counter++
            stringBuilder.append(if (!shortenedOutput) attribution.title else attribution.titleAbbreviated)
            if (counter != attributions.size) {
                stringBuilder.append(" / ")
            }
        }
        return stringBuilder.toString()
    }

    /**
     * Main attribution for configuration
     */
    private fun parse() {
        parseAttributions()
    }

    /**
     * Parse attributions
     */
    private fun parseAttributions() {
        val htmlBuilder = fromHtml(attributionData) as SpannableStringBuilder
        val urlSpans = htmlBuilder.getSpans(0, htmlBuilder.length, URLSpan::class.java)
        for (urlSpan in urlSpans) {
            parseUrlSpan(htmlBuilder, urlSpan)
        }
    }

    /**
     * Parse an URLSpan containing an attribution.
     *
     * @param htmlBuilder the html builder
     * @param urlSpan     the url span to be parsed
     */
    private fun parseUrlSpan(
        htmlBuilder: SpannableStringBuilder,
        urlSpan: URLSpan,
    ) {
        val url = urlSpan.url
        if (isUrlValid(url)) {
            val anchor = parseAnchorValue(htmlBuilder, urlSpan)
            attributionSet.add(Attribution(anchor, url))
        }
    }

    /**
     * Invoked to validate if an url is valid to be included in the final attribution.
     *
     * @param url the url to be validated
     * @return if the url is valid
     */
    private fun isUrlValid(url: String): Boolean = isValidForImproveThisMap(url) && isValidForMapbox(url)

    /**
     * Invoked to validate if an url is valid for the improve map configuration.
     *
     * @param url the url to be validated
     * @return if the url is valid for improve this map
     */
    private fun isValidForImproveThisMap(url: String): Boolean = withImproveMap || !Attribution.IMPROVE_MAP_URLS.contains(url)

    /**
     * Invoked to validate if an url is valid for the MapLibre configuration.
     *
     * @param url the url to be validated
     * @return if the url is valid for MapLibre
     */
    private fun isValidForMapbox(url: String): Boolean = withMapboxAttribution || url != Attribution.MAPBOX_URL

    /**
     * Parse the attribution by parsing the anchor value of html href tag.
     *
     * @param htmlBuilder the html builder
     * @param urlSpan     the current urlSpan
     * @return the parsed anchor value
     */
    private fun parseAnchorValue(
        htmlBuilder: SpannableStringBuilder,
        urlSpan: URLSpan,
    ): String {
        val start = htmlBuilder.getSpanStart(urlSpan)
        val end = htmlBuilder.getSpanEnd(urlSpan)
        val length = end - start
        val charKey = CharArray(length)
        htmlBuilder.getChars(start, end, charKey, 0)
        return stripCopyright(String(charKey))
    }

    /**
     * Utility to strip the copyright sign from an attribution
     *
     * @param anchor the attribution string to strip
     * @return the stripped attribution string without the copyright sign
     */
    private fun stripCopyright(anchor: String): String {
        if (!withCopyrightSign && anchor.startsWith("© ")) {
            return anchor.substring(2, anchor.length)
        }
        return anchor
    }

    /**
     * Builder to configure using an AttributionParser.
     *
     * AttributionData, set with [withAttributionData], is the only required property to build
     * the underlying AttributionParser. Other properties include trimming the copyright sign, hiding
     * attribution as improve this map and MapLibre.
     */
    class Options(
        context: Context,
    ) {
        private val context: WeakReference<Context> = WeakReference(context)
        private var withImproveMap = true
        private var withCopyrightSign = true
        private var withMapboxAttribution = true
        private var attributionDataStringArray: Array<out String>? = null

        fun withAttributionData(vararg attributionData: String): Options {
            this.attributionDataStringArray = attributionData
            return this
        }

        fun withImproveMap(withImproveMap: Boolean): Options {
            this.withImproveMap = withImproveMap
            return this
        }

        fun withCopyrightSign(withCopyrightSign: Boolean): Options {
            this.withCopyrightSign = withCopyrightSign
            return this
        }

        fun withMapboxAttribution(withMapboxAttribution: Boolean): Options {
            this.withMapboxAttribution = withMapboxAttribution
            return this
        }

        fun build(): AttributionParser {
            val attributionData =
                attributionDataStringArray
                    ?: throw IllegalStateException("Using builder without providing attribution data")

            val fullAttributionString = parseAttribution(attributionData)
            val attributionParser =
                AttributionParser(
                    context,
                    fullAttributionString,
                    withImproveMap,
                    withCopyrightSign,
                    withMapboxAttribution,
                )
            attributionParser.parse()
            return attributionParser
        }

        private fun parseAttribution(attribution: Array<out String>): String {
            val builder = StringBuilder()
            for (attr in attribution) {
                if (attr.isNotEmpty()) {
                    builder.append(attr)
                }
            }
            return builder.toString()
        }
    }

    private companion object {
        /**
         * Convert a string to a spanned html representation.
         *
         * @param html the string to convert
         * @return the spanned html representation
         */
        @Suppress("DEPRECATION")
        fun fromHtml(html: String): Spanned =
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                Html.fromHtml(html, Html.FROM_HTML_MODE_LEGACY)
            } else {
                Html.fromHtml(html)
            }
    }
}
