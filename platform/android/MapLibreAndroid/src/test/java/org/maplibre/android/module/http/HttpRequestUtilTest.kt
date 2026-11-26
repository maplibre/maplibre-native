package org.maplibre.android.module.http

import org.maplibre.android.MapLibreInjector
import org.maplibre.android.utils.ConfigUtils
import io.mockk.mockk
import okhttp3.Call
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class HttpRequestUtilTest : BaseTest() {

    @Test
    fun replaceHttpClient() {
        MapLibreInjector.inject(mockk(relaxed = true), "", ConfigUtils.getMockedOptions())

        val httpMock = mockk<Call.Factory>()
        HttpRequestUtil.setOkHttpClient(httpMock)

        assertNull("Default HTTP client should not be initialized", HttpRequestImpl.defaultClient)
        assertEquals(
            "Http client should have set to the mocked client",
            httpMock,
            HttpRequestImpl.client
        )

        HttpRequestUtil.setOkHttpClient(null)
        assertNotNull("Default HTTP client should be initialized", HttpRequestImpl.defaultClient)
        assertEquals(
            "Http client should have been reset to the default client",
            HttpRequestImpl.defaultClient,
            HttpRequestImpl.client
        )

        MapLibreInjector.clear()
    }
}
