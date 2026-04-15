package org.maplibre.android.location

import android.content.Context
import android.content.res.Resources
import android.content.res.TypedArray
import android.graphics.Color
import android.view.animation.LinearInterpolator
import org.maplibre.android.R
import org.maplibre.android.maps.Style
import org.junit.Assert
import org.junit.Before
import org.junit.Rule
import org.junit.Test
import org.junit.rules.ExpectedException
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.mockito.Mock
import org.mockito.Mockito
import org.mockito.junit.MockitoJUnit
import org.mockito.junit.MockitoJUnitRunner

@RunWith(MockitoJUnitRunner::class)
class LocationComponentActivationOptionsTest : BaseTest() {
    @Mock
    private val context: Context? = null

    @Mock
    private val array: TypedArray? = null

    @Mock
    private val resources: Resources? = null

    @Mock
    private val style: Style? = null

    @Rule @JvmField
    var mockitoRule = MockitoJUnit.rule()

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
        Mockito.`when`(style!!.isFullyLoaded).thenReturn(true)
        val locationComponentOptions = LocationComponentOptions.builder(
            context!!
        )
            .accuracyAlpha(0.5f)
            .build()
        Assert.assertNotNull(locationComponentOptions)
        val locationComponentActivationOptions = LocationComponentActivationOptions.builder(
            context,
            style
        )
            .locationComponentOptions(locationComponentOptions)
            .useDefaultLocationEngine(true)
            .build()
        Assert.assertNotNull(locationComponentActivationOptions)
    }

    @Test
    @Throws(Exception::class)
    fun sanityWithDefaultPulsingCircle() {
        Mockito.`when`(style!!.isFullyLoaded).thenReturn(true)
        val locationComponentOptions = LocationComponentOptions.builder(
            context!!
        )
            .accuracyAlpha(0.5f)
            .pulseEnabled(true)
            .build()
        Assert.assertNotNull(locationComponentOptions)
        val locationComponentActivationOptions = LocationComponentActivationOptions.builder(
            context,
            style
        )
            .locationComponentOptions(locationComponentOptions)
            .useDefaultLocationEngine(true)
            .build()
        Assert.assertNotNull(locationComponentActivationOptions)
    }

    @Test
    @Throws(Exception::class)
    fun sanityWithCustomizedPulsingCircle() {
        Mockito.`when`(style!!.isFullyLoaded).thenReturn(true)
        val locationComponentOptions = LocationComponentOptions.builder(
            context!!
        )
            .accuracyAlpha(0.5f)
            .pulseEnabled(true)
            .pulseColor(Color.RED)
            .pulseInterpolator(LinearInterpolator())
            .build()
        Assert.assertNotNull(locationComponentOptions)
        val locationComponentActivationOptions = LocationComponentActivationOptions.builder(
            context,
            style
        )
            .locationComponentOptions(locationComponentOptions)
            .useDefaultLocationEngine(true)
            .build()
        Assert.assertNotNull(locationComponentActivationOptions)
    }

    @Test
    @Throws(Exception::class)
    fun includingBothStyleResAndComponentOptions_causesExceptionToBeThrown() {
        thrown.expect(IllegalArgumentException::class.java)
        thrown.expectMessage(
            "You've provided both a style resource and a LocationComponentOptions" +
                " object to the LocationComponentActivationOptions builder. You can't use both and " +
                "you must choose one of the two to style the LocationComponent."
        )
        val locationComponentOptions = LocationComponentOptions.builder(
            context!!
        )
            .accuracyAlpha(0.5f)
            .build()
        LocationComponentActivationOptions.builder(context, style!!)
            .locationComponentOptions(locationComponentOptions)
            .styleRes(R.style.maplibre_LocationComponent)
            .build()
    }

    @Test
    @Throws(Exception::class)
    fun locationComponent_exceptionThrownWithDefaultLocationEngineButNotFullyLoadedStyle() {
        Mockito.`when`(style!!.isFullyLoaded).thenReturn(false)
        thrown.expect(IllegalArgumentException::class.java)
        thrown.expectMessage(
            "Style in LocationComponentActivationOptions isn't fully loaded. Wait for the " +
                "map to fully load before passing the Style object to " +
                "LocationComponentActivationOptions."
        )
        LocationComponentActivationOptions.builder(context!!, style)
            .build()
    }
}
