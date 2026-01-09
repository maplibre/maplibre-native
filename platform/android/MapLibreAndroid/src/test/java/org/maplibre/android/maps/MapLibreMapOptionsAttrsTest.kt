package org.maplibre.android.maps

import android.content.Context
import android.content.res.Resources
import android.content.res.TypedArray
import org.maplibre.android.R
import io.mockk.MockKAnnotations
import io.mockk.every
import io.mockk.impl.annotations.RelaxedMockK
import io.mockk.verify
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class MapLibreMapOptionsAttrsTest : BaseTest() {

    @RelaxedMockK
    private lateinit var options: MapLibreMapOptions

    @RelaxedMockK
    private lateinit var typedArray: TypedArray

    @RelaxedMockK
    private lateinit var context: Context

    @RelaxedMockK
    private lateinit var resources: Resources

    @Before
    fun setUp() {
        MockKAnnotations.init(this)
        every {
            context.resources
        }.returns(resources)
    }

    @Test
    fun enabledLocalIdeographFontFamily() {
        mockEnableLocalIdeograph(enabled = true)

        val options = MapLibreMapOptions.createFromAttributes(options, context, typedArray)

        verify(exactly = 1) {
            options.localIdeographFontFamily(any())
        }
    }

    @Test
    fun localIdeographFontFamily() {
        mockEnableLocalIdeograph(enabled = true)

        val font = "foo"
        mockLocalIdeographString(font)

        val options = MapLibreMapOptions.createFromAttributes(options, context, typedArray)

        verify(exactly = 1) {
            options.localIdeographFontFamily(font)
        }
    }

    @Test
    fun localIdeographFontFamilies() {
        mockEnableLocalIdeograph(enabled = true)

        val fonts = arrayOf("foo", "bar")
        mockLocalIdeographStringArray(fonts)

        val options = MapLibreMapOptions.createFromAttributes(options, context, typedArray)

        verify(exactly = 1) {
            options.localIdeographFontFamily(*fonts)
        }
    }

    private fun mockEnableLocalIdeograph(enabled: Boolean) {
        every {
            typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_localIdeographEnabled, true)
        }.returns(enabled)
    }

    private fun mockLocalIdeographString(font: String) {
        every {
            typedArray.getString(R.styleable.maplibre_MapView_maplibre_localIdeographFontFamily)
        }.returns(font)
    }

    private fun mockLocalIdeographStringArray(fonts: Array<String>) {
        val resId = 9000

        every {
            typedArray.getResourceId(R.styleable.maplibre_MapView_maplibre_localIdeographFontFamilies, 0)
        }.returns(resId)

        every {
            resources.getStringArray(resId)
        }.returns(fonts)
    }
}
