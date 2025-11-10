package org.maplibre.android.attribution

import org.junit.Assert
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.robolectric.RobolectricTestRunner
import org.robolectric.RuntimeEnvironment

@RunWith(RobolectricTestRunner::class)
class AttributionParseTest : BaseTest() {
    @Test
    @Throws(Exception::class)
    fun testParseAttributionStringSatellite() {
        val attributionParser = AttributionParser.Options(RuntimeEnvironment.application)
            .withAttributionData(SATELLITE_ATTRIBUTION)
            .build()
        val attributionList = attributionParser.attributions
        Assert.assertEquals("Size of list should match", 3, attributionList.size)
        for ((counter, attribution) in attributionList.withIndex()) {
            when (counter) {
                0 -> {
                    Assert.assertEquals(
                        "URL mapbox should match",
                        "https://www.mapbox.com/about/maps/",
                        attribution.url
                    )
                    Assert.assertEquals("Title mapbox should match", "© MapLibre", attribution.title)
                }
                1 -> {
                    Assert.assertEquals(
                        "URL openstreetmap should match",
                        "http://www.openstreetmap.org/about/",
                        attribution.url
                    )
                    Assert.assertEquals(
                        "Title openstreetmap should match",
                        "© OpenStreetMap",
                        attribution.title
                    )
                }
                2 -> {
                    Assert.assertEquals(
                        "URL digital globe should match",
                        "https://www.digitalglobe.com/",
                        attribution.url
                    )
                    Assert.assertEquals(
                        "Title digital globe should match",
                        "© DigitalGlobe",
                        attribution.title
                    )
                }
            }
        }
    }

    @Test
    @Throws(Exception::class)
    fun testParseAttributionStringStreets() {
        val attributionParser = AttributionParser.Options(RuntimeEnvironment.application)
            .withAttributionData(STREETS_ATTRIBUTION)
            .build()
        val attributionList = attributionParser.attributions
        Assert.assertEquals("Size of list should match", 2, attributionList.size)
        for ((counter, attribution) in attributionList.withIndex()) {
            when (counter) {
                0 -> {
                    Assert.assertEquals(
                        "URL mapbox should match",
                        "https://www.mapbox.com/about/maps/",
                        attribution.url
                    )
                    Assert.assertEquals("Title mapbox should match", "© MapLibre", attribution.title)
                }
                1 -> {
                    Assert.assertEquals(
                        "URL openstreetmap should match",
                        "http://www.openstreetmap.org/about/",
                        attribution.url
                    )
                    Assert.assertEquals(
                        "Title openstreetmap should match",
                        "© OpenStreetMap",
                        attribution.title
                    )
                }
            }
        }
    }

    @Test
    @Throws(Exception::class)
    fun testParseAttributionWithoutMapbox() {
        val attributionParser = AttributionParser.Options(RuntimeEnvironment.application)
            .withAttributionData(STREETS_ATTRIBUTION)
            .withMapboxAttribution(false)
            .build()
        val attributionList = attributionParser.attributions
        Assert.assertEquals("Size of list should match", 1, attributionList.size)
        for ((counter, attribution) in attributionList.withIndex()) {
            when (counter) {
                0 -> {
                    Assert.assertEquals(
                        "URL openstreetmap should match",
                        "http://www.openstreetmap.org/about/",
                        attribution.url
                    )
                    Assert.assertEquals(
                        "Title openstreetmap should match",
                        "© OpenStreetMap",
                        attribution.title
                    )
                }
            }
        }
    }

    @Test
    @Throws(Exception::class)
    fun testParseAttributionArrayString() {
        val attributionParser = AttributionParser.Options(RuntimeEnvironment.application)
            .withAttributionData(*arrayOf(STREETS_ATTRIBUTION, "", SATELLITE_ATTRIBUTION))
            .build()
        val attributionList = attributionParser.attributions
        Assert.assertEquals("Size of list should match", 3, attributionList.size)
        for ((counter, attribution) in attributionList.withIndex()) {
            when (counter) {
                0 -> {
                    Assert.assertEquals(
                        "URL mapbox should match",
                        "https://www.mapbox.com/about/maps/",
                        attribution.url
                    )
                    Assert.assertEquals("Title mapbox should match", "© MapLibre", attribution.title)
                }
                1 -> {
                    Assert.assertEquals(
                        "URL openstreetmap should match",
                        "http://www.openstreetmap.org/about/",
                        attribution.url
                    )
                    Assert.assertEquals(
                        "Title openstreetmap should match",
                        "© OpenStreetMap",
                        attribution.title
                    )
                }
                2 -> {
                    Assert.assertEquals(
                        "URL digital globe should match",
                        "https://www.digitalglobe.com/",
                        attribution.url
                    )
                    Assert.assertEquals(
                        "Title digital globe should match",
                        "© DigitalGlobe",
                        attribution.title
                    )
                }
            }
        }
    }

    @Test
    @Throws(Exception::class)
    fun testHideImproveThisMapAttributionArrayString() {
        val attributionParser = AttributionParser.Options(RuntimeEnvironment.application)
            .withAttributionData(SATELLITE_ATTRIBUTION)
            .withImproveMap(false)
            .build()
        val attributionList = attributionParser.attributions
        Assert.assertEquals("Size of list should match", 3, attributionList.size)
        for ((counter, attribution) in attributionList.withIndex()) {
            when (counter) {
                0 -> {
                    Assert.assertEquals(
                        "URL mapbox should match",
                        "https://www.mapbox.com/about/maps/",
                        attribution.url
                    )
                    Assert.assertEquals("Title mapbox should match", "© MapLibre", attribution.title)
                }
                1 -> {
                    Assert.assertEquals(
                        "URL openstreetmap should match",
                        "http://www.openstreetmap.org/about/",
                        attribution.url
                    )
                    Assert.assertEquals(
                        "Title openstreetmap should match",
                        "© OpenStreetMap",
                        attribution.title
                    )
                }
                2 -> {
                    Assert.assertEquals(
                        "URL digital globe should match",
                        "https://www.digitalglobe.com/",
                        attribution.url
                    )
                    Assert.assertEquals(
                        "Title digital globe should match",
                        "© DigitalGlobe",
                        attribution.title
                    )
                }
            }
        }
    }

    @Test
    @Throws(Exception::class)
    fun testParseHideCopyrightAttributionArrayString() {
        val attributionParser = AttributionParser.Options(RuntimeEnvironment.application)
            .withAttributionData(STREETS_ATTRIBUTION, "", SATELLITE_ATTRIBUTION)
            .withCopyrightSign(false)
            .build()
        val attributionList = attributionParser.attributions
        Assert.assertEquals("Size of list should match", 3, attributionList.size)
        for ((counter, attribution) in attributionList.withIndex()) {
            when (counter) {
                0 -> {
                    Assert.assertEquals(
                        "URL mapbox should match",
                        "https://www.mapbox.com/about/maps/",
                        attribution.url
                    )
                    Assert.assertEquals("Title mapbox should match", "MapLibre", attribution.title)
                }
                1 -> {
                    Assert.assertEquals(
                        "URL openstreetmap should match",
                        "http://www.openstreetmap.org/about/",
                        attribution.url
                    )
                    Assert.assertEquals(
                        "Title openstreetmap should match",
                        "OpenStreetMap",
                        attribution.title
                    )
                }
                2 -> {
                    Assert.assertEquals(
                        "URL digital globe should match",
                        "https://www.digitalglobe.com/",
                        attribution.url
                    )
                    Assert.assertEquals(
                        "Title digital globe should match",
                        "DigitalGlobe",
                        attribution.title
                    )
                }
            }
        }
    }

    @Test
    @Throws(Exception::class)
    fun testOutputWithoutCopyRightString() {
        val attributionParser = AttributionParser.Options(RuntimeEnvironment.application)
            .withAttributionData(STREETS_ATTRIBUTION)
            .withCopyrightSign(false)
            .withImproveMap(false)
            .build()
        Assert.assertEquals(
            "Attribution string should match",
            "© MapLibre / OpenStreetMap",
            attributionParser.createAttributionString()
        )
    }

    @Test
    @Throws(Exception::class)
    fun testOutputWithCopyRightString() {
        val attributionParser = AttributionParser.Options(RuntimeEnvironment.application)
            .withAttributionData(STREETS_ATTRIBUTION)
            .withImproveMap(false)
            .build()
        Assert.assertEquals(
            "Attribution string should match",
            "© MapLibre / © OpenStreetMap",
            attributionParser.createAttributionString()
        )
    }

    @Test
    @Throws(Exception::class)
    fun testOutputWithoutCopyRightWithoutMapboxString() {
        val attributionParser = AttributionParser.Options(RuntimeEnvironment.application)
            .withAttributionData(STREETS_ATTRIBUTION)
            .withCopyrightSign(false)
            .withImproveMap(false)
            .withMapboxAttribution(false)
            .build()
        Assert.assertEquals(
            "Attribution string should match",
            "© OpenStreetMap",
            attributionParser.createAttributionString()
        )
    }

    @Test
    @Throws(Exception::class)
    fun testOutputWithCopyRightWithoutMapboxString() {
        val attributionParser = AttributionParser.Options(RuntimeEnvironment.application)
            .withAttributionData(STREETS_ATTRIBUTION)
            .withImproveMap(false)
            .withMapboxAttribution(false)
            .build()
        Assert.assertEquals(
            "Attribution string should match",
            "© OpenStreetMap",
            attributionParser.createAttributionString()
        )
    }

    @Test
    @Throws(Exception::class)
    fun testOutputSatelliteString() {
        val attributionParser = AttributionParser.Options(RuntimeEnvironment.application)
            .withAttributionData(STREETS_ATTRIBUTION, SATELLITE_ATTRIBUTION, "blabla", "")
            .withImproveMap(false)
            .withCopyrightSign(false)
            .withMapboxAttribution(false)
            .build()
        Assert.assertEquals(
            "Attribution string should match",
            "© OpenStreetMap / DigitalGlobe",
            attributionParser.createAttributionString()
        )
    }

    @Test
    @Throws(Exception::class)
    fun testShortOpenStreetMapString() {
        val attributionParser = AttributionParser.Options(RuntimeEnvironment.application)
            .withAttributionData(STREETS_ATTRIBUTION, SATELLITE_ATTRIBUTION, "blabla", "")
            .withImproveMap(false)
            .withCopyrightSign(false)
            .withMapboxAttribution(false)
            .build()
        Assert.assertEquals(
            "Attribution string should match",
            "© OSM / DigitalGlobe",
            attributionParser.createAttributionString(true)
        )
    }

    @Test
    @Throws(Exception::class)
    fun testShortOpenStreetMapWithoutCopyrightString() {
        val attributionParser = AttributionParser.Options(RuntimeEnvironment.application)
            .withAttributionData(STREETS_ATTRIBUTION, SATELLITE_ATTRIBUTION, "blabla", "")
            .withImproveMap(false)
            .withCopyrightSign(false)
            .build()
        Assert.assertEquals(
            "Attribution string should match",
            "© MapLibre / OSM / DigitalGlobe",
            attributionParser.createAttributionString(true)
        )
    }

    @Test
    @Throws(Exception::class)
    fun testOutputNoAttribution() {
        val attributionParser = AttributionParser.Options(RuntimeEnvironment.application)
            .withAttributionData("")
            .withCopyrightSign(false)
            .withImproveMap(false)
            .build()
        Assert.assertEquals(
            "Attribution string should match",
            "",
            attributionParser.createAttributionString()
        )
    }

    companion object {
        private const val STREETS_ATTRIBUTION =
            "<a href=\"https://www.mapbox.com/about/maps/\" target=\"_blank\">&copy; MapLibre</a> <a href=\"http://www.openstreetmap.org/about/\" target=\"_blank\">&copy; OpenStreetMap</a> \n"
        private const val SATELLITE_ATTRIBUTION =
            "<a href=\"https://www.mapbox.com/about/maps/\" target=\"_blank\">&copy; MapLibre</a> <a href=\"http://www.openstreetmap.org/about/\" target=\"_blank\">&copy; OpenStreetMap</a> <a href=\"https://www.digitalglobe.com/\" target=\"_blank\">&copy; DigitalGlobe</a>\n"
    }
}
