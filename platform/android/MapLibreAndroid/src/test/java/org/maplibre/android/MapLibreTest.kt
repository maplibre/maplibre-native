package org.maplibre.android

import android.content.Context
import android.content.res.Resources
import android.content.res.TypedArray
import android.util.AttributeSet
import android.util.DisplayMetrics
import org.junit.After
import org.junit.Assert
import org.junit.Before
import org.junit.Rule
import org.junit.Test
import org.junit.rules.ExpectedException
import org.maplibre.android.MapLibreInjector.clear
import org.maplibre.android.MapLibreInjector.inject
import org.maplibre.android.exceptions.MapLibreConfigurationException
import org.maplibre.android.maps.MapView
import org.maplibre.android.utils.ConfigUtils.Companion.getMockedOptions
import org.mockito.ArgumentMatchers
import org.mockito.Mockito
import java.io.File

class MapLibreTest : BaseTest() {
    private var context: Context? = null
    private var appContext: Context? = null

    @Rule
    @JvmField // J2K: https://stackoverflow.com/a/33449455
    var expectedException = ExpectedException.none()
    @Before
    fun before() {
        context = Mockito.mock(Context::class.java)
        appContext = Mockito.mock(Context::class.java)
        // J2K: https://www.baeldung.com/kotlin/smart-cast-to-type-is-impossible#2-using-the-safe-call-operator--and-a-scope-function
        Mockito.`when`(context?.getApplicationContext()).thenReturn(appContext)
    }

    @Test
    fun testGetApiKey() {
        val apiKey = "pk.0000000001"
        inject(context!!, apiKey, getMockedOptions())
        Assert.assertSame(apiKey, MapLibre.getApiKey())
    }

    @Test
    fun testApplicationContext() {
        inject(context!!, "pk.0000000001", getMockedOptions())
        Assert.assertNotNull(MapLibre.getApplicationContext())
        Assert.assertNotEquals(context, appContext)
        Assert.assertEquals(appContext, appContext)
    }

    @Test
    fun testPlainTokenValid() {
        Assert.assertTrue(MapLibre.isApiKeyValid("apiKey"))
    }

    @Test
    fun testEmptyToken() {
        Assert.assertFalse(MapLibre.isApiKeyValid(""))
    }

    @Test
    fun testNullToken() {
        Assert.assertFalse(MapLibre.isApiKeyValid(null))
    }

    @Test
    fun testNoInstance() {
        val displayMetrics = Mockito.mock(DisplayMetrics::class.java)
        val resources = Mockito.mock(Resources::class.java)
        val files = Mockito.mock(File::class.java)
        Mockito.`when`(resources.displayMetrics).thenReturn(displayMetrics)
        Mockito.`when`(context!!.resources).thenReturn(resources)
        Mockito.`when`(context!!.filesDir).thenReturn(files)
        val typedArray = Mockito.mock(TypedArray::class.java)
        Mockito.`when`(context!!.obtainStyledAttributes(ArgumentMatchers.nullable(AttributeSet::class.java), ArgumentMatchers.any(IntArray::class.java), ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt()))
                .thenReturn(typedArray)
        expectedException.expect(MapLibreConfigurationException::class.java)
        expectedException.expectMessage("""

    Using MapView requires calling MapLibre.getInstance(Context context, String apiKey, WellKnownTileServer wellKnownTileServer) before inflating or creating the view.
    """.trimIndent())
        MapView(context!!)
    }

    @After
    fun after() {
        clear()
    }
}
