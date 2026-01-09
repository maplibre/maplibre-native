package org.maplibre.android.maps

import android.content.Context
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.mockito.InjectMocks
import org.mockito.Mockito
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class AttributionDialogManagerTest : BaseTest() {
    @InjectMocks
    var context = Mockito.mock(
        Context::class.java
    )

    @InjectMocks
    var maplibreMap = Mockito.mock(MapLibreMap::class.java)

    @InjectMocks
    var style = Mockito.mock(
        Style::class.java
    )
    private var attributionDialogManager: AttributionDialogManager? = null

    @Before
    fun beforeTest() {
        attributionDialogManager = AttributionDialogManager(context, maplibreMap)
    }

    @Test
    fun testSanity() {
        Assert.assertNotNull(
            "AttributionDialogManager should not be null",
            attributionDialogManager
        )
    }
}
