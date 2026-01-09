package org.maplibre.android.location

import android.content.Context
import android.content.res.Resources
import android.content.res.TypedArray
import org.maplibre.android.R
import org.junit.Assert
import org.junit.Before
import org.junit.Rule
import org.junit.Test
import org.junit.rules.ExpectedException
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.mockito.Mock
import org.mockito.Mockito
import org.mockito.junit.MockitoJUnitRunner

@RunWith(MockitoJUnitRunner::class)
class LocationComponentOptionsTest : BaseTest() {
    @Mock
    private val context: Context? = null

    @Mock
    private val array: TypedArray? = null

    @Mock
    private val resources: Resources? = null

    @Rule @JvmField
    var thrown = ExpectedException.none()

    @Before
    @Throws(Exception::class)
    fun setUp() {
        Mockito.`when`(
            context!!.obtainStyledAttributes(
                R.style.maplibre_LocationComponent,
                R.styleable.maplibre_LocationComponent
            )
        )
            .thenReturn(array)
        Mockito.`when`(
            array!!.getResourceId(
                R.styleable.maplibre_LocationComponent_maplibre_foregroundDrawable,
                -1
            )
        )
            .thenReturn(R.drawable.maplibre_user_icon)
        Mockito.`when`(context.resources).thenReturn(resources)
    }

    @Test
    @Throws(Exception::class)
    fun sanity() {
        val locationComponentOptions = LocationComponentOptions.builder(
            context!!
        )
            .accuracyAlpha(0.5f)
            .build()
        Assert.assertNotNull(locationComponentOptions)
    }

    @Test
    @Throws(Exception::class)
    fun passingOutOfRangeAccuracyAlpha_throwsException() {
        thrown.expect(IllegalArgumentException::class.java)
        thrown.expectMessage(
            "Accuracy alpha value must be between 0.0 and " +
                "1.0."
        )
        LocationComponentOptions.builder(context!!)
            .accuracyAlpha(2f)
            .build()
    }

    @Test
    @Throws(Exception::class)
    fun negativeElevation_causesExceptionToBeThrown() {
        thrown.expect(IllegalArgumentException::class.java)
        thrown.expectMessage("Invalid shadow size -500.0. Must be >= 0")
        LocationComponentOptions.builder(context!!)
            .elevation(-500f)
            .build()
    }

    @Test
    @Throws(Exception::class)
    fun passingBothLayerPositionOptions_throwsException() {
        thrown.expect(IllegalArgumentException::class.java)
        thrown.expectMessage(
            "You cannot set both layerAbove and layerBelow options." +
                " Choose one or the other."
        )
        LocationComponentOptions.builder(context!!)
            .layerAbove("above")
            .layerBelow("below")
            .build()
    }
}
