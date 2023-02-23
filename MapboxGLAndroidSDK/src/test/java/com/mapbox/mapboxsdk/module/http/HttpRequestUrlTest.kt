package com.mapbox.mapboxsdk.module.http

import com.mapbox.mapboxsdk.MapboxInjector
import com.mapbox.mapboxsdk.http.HttpRequestUrl
import com.mapbox.mapboxsdk.utils.ConfigUtils
import io.mockk.mockk
import junit.framework.Assert.assertEquals
import org.junit.After
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class HttpRequestUrlTest {

    @Before
    fun setUp() {
        MapboxInjector.inject(mockk(relaxed = true), "pk.foobar", ConfigUtils.getMockedOptions())
    }

    @Test
    fun testOfflineFlagMapboxCom() {
        val expected = "http://mapbox.com/path/of/no/return.pbf?offline=true"
        val actual = HttpRequestUrl.buildResourceUrl("mapbox.com", "http://mapbox.com/path/of/no/return.pbf", 0, true)
        assertEquals(expected, actual)
    }

    @Test
    fun testOfflineFlagMapboxCn() {
        val expected = "http://mapbox.cn/path/of/no/return.pbf?offline=true"
        val actual = HttpRequestUrl.buildResourceUrl("mapbox.cn", "http://mapbox.cn/path/of/no/return.pbf", 0, true)
        assertEquals(expected, actual)
    }

    @Test
    fun testOfflineFlagInvalidHost() {
        val expected = "http://foobar.com/path/of/no/return.pbf"
        val actual = HttpRequestUrl.buildResourceUrl("foobar.com", "http://foobar.com/path/of/no/return.pbf", 0, true)
        assertEquals(expected, actual)
    }

    @Test
    fun testOnlineInvalidHost() {
        val expected = "http://foobar.com/path/of/no/return.pbf"
        val actual = HttpRequestUrl.buildResourceUrl("foobar.com", "http://foobar.com/path/of/no/return.pbf", 0, false)
        assertEquals(expected, actual)
    }

    @After
    fun tearDown() {
        MapboxInjector.clear()
    }
}
