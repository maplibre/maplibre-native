package org.maplibre.android.module.http

import org.maplibre.android.MapLibreInjector
import org.maplibre.android.utils.ConfigUtils
import io.mockk.mockk
import okhttp3.Call
import org.junit.Assert.assertEquals
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class HttpRequestUtilTest : BaseTest() {

    @Test
    fun replaceHttpClient() {
        MapLibreInjector.inject(mockk(relaxed = true), "", ConfigUtils.getMockedOptions())

        assertEquals(HttpRequestImpl.DEFAULT_CLIENT, HttpRequestImpl.client)

        val httpMock = mockk<Call.Factory>()
        HttpRequestUtil.setOkHttpClient(httpMock)
        assertEquals(
            "Http client should have set to the mocked client",
            httpMock,
            HttpRequestImpl.client
        )

        HttpRequestUtil.setOkHttpClient(null)
        assertEquals(
            "Http client should have been reset to the default client",
            HttpRequestImpl.DEFAULT_CLIENT,
            HttpRequestImpl.client
        )

        MapLibreInjector.clear()
    }
}
