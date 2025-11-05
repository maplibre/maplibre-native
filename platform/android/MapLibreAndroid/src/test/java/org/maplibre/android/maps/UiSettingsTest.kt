package org.maplibre.android.maps

import android.content.Context
import android.content.res.Resources
import android.view.Gravity
import android.view.View
import android.widget.FrameLayout
import android.widget.ImageView
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.maps.widgets.CompassView
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.maplibre.android.BaseTest
import org.mockito.InjectMocks
import org.mockito.Mockito

class UiSettingsTest : BaseTest() {
    @InjectMocks
    var mapview = Mockito.mock(MapView::class.java)

    @InjectMocks
    var context = Mockito.mock(
        Context::class.java
    )

    @InjectMocks
    var resources = Mockito.mock(
        Resources::class.java
    )

    @InjectMocks
    var projection = Mockito.mock(
        Projection::class.java
    )

    @InjectMocks
    var focalPointChangeListener = Mockito.mock(
        FocalPointChangeListener::class.java
    )

    @InjectMocks
    var compassView = Mockito.mock(CompassView::class.java)

    @InjectMocks
    var attributionView = Mockito.mock(
        ImageView::class.java
    )

    @InjectMocks
    var logoView = Mockito.mock(
        ImageView::class.java
    )

    @InjectMocks
    var layoutParams = Mockito.mock(
        FrameLayout.LayoutParams::class.java
    )
    private var uiSettings: UiSettings? = null
    private val maplibreMapOptions: MapLibreMapOptions? = null

    @Before
    fun beforeTest() {
        uiSettings = UiSettings(projection, focalPointChangeListener, 1f, mapview)
        uiSettings!!.compassView = compassView
        uiSettings!!.attributionsView = attributionView
        uiSettings!!.logoView = logoView
        Mockito.`when`(compassView.context).thenReturn(context)
        Mockito.`when`(attributionView.context).thenReturn(context)
        Mockito.`when`(logoView.context).thenReturn(context)
        Mockito.`when`(context.resources).thenReturn(resources)
    }

    @Test
    fun testSanity() {
        Assert.assertNotNull("uiSettings should not be null", uiSettings)
    }

    @Test
    fun testGetCompassEnabled() {
        Mockito.`when`(compassView.isEnabled).thenReturn(true)
        Assert.assertEquals("Compass should be enabled", true, uiSettings!!.isCompassEnabled)
    }

    @Test
    fun testSetCompassEnabled() {
        uiSettings!!.isCompassInitialized = true
        uiSettings!!.isCompassEnabled = true
        Mockito.verify(compassView).isEnabled = true
    }

    @Test
    fun testCompassDisabled() {
        uiSettings!!.isCompassEnabled = false
        Assert.assertEquals("Compass should be disabled", false, uiSettings!!.isCompassEnabled)
    }

    @Test
    fun testCompassGravity() {
        Mockito.`when`(compassView.layoutParams).thenReturn(layoutParams)
        layoutParams.gravity = Gravity.START
        uiSettings!!.compassGravity = Gravity.START
        Assert.assertEquals(
            "Compass gravity should be same",
            Gravity.START.toLong(),
            uiSettings!!.compassGravity.toLong()
        )
    }

    @Test
    fun testCompassMargins() {
        Mockito.`when`(projection.contentPadding).thenReturn(intArrayOf(0, 0, 0, 0))
        Mockito.`when`(compassView.layoutParams).thenReturn(layoutParams)
        layoutParams.leftMargin = 1
        layoutParams.topMargin = 2
        layoutParams.rightMargin = 3
        layoutParams.bottomMargin = 4
        uiSettings!!.setCompassMargins(1, 2, 3, 4)
        Assert.assertTrue("Compass margin left should be same", uiSettings!!.compassMarginLeft == 1)
        Assert.assertTrue("Compass margin top should be same", uiSettings!!.compassMarginTop == 2)
        Assert.assertTrue(
            "Compass margin right should be same",
            uiSettings!!.compassMarginRight == 3
        )
        Assert.assertTrue(
            "Compass margin bottom should be same",
            uiSettings!!.compassMarginBottom == 4
        )
    }

    @Test
    fun testCompassFadeWhenFacingNorth() {
        Mockito.`when`(compassView.isFadeCompassViewFacingNorth).thenReturn(true)
        Assert.assertTrue(
            "Compass should fade when facing north by default.",
            uiSettings!!.isCompassFadeWhenFacingNorth
        )
        uiSettings!!.setCompassFadeFacingNorth(false)
        Mockito.`when`(compassView.isFadeCompassViewFacingNorth).thenReturn(false)
        Assert.assertFalse(
            "Compass fading should be disabled",
            uiSettings!!.isCompassFadeWhenFacingNorth
        )
    }

    @Test
    fun testLogoEnabled() {
        uiSettings!!.isLogoInitialized = true
        uiSettings!!.isLogoEnabled = true
        Assert.assertEquals("Logo should be enabled", true, uiSettings!!.isLogoEnabled)
    }

    @Test
    fun testLogoDisabled() {
        Mockito.`when`(logoView.visibility).thenReturn(View.GONE)
        uiSettings!!.isLogoEnabled = false
        Assert.assertEquals("Logo should be disabled", false, uiSettings!!.isLogoEnabled)
    }

    @Test
    fun testLogoGravity() {
        layoutParams.gravity = Gravity.END
        Mockito.`when`(logoView.layoutParams).thenReturn(layoutParams)
        uiSettings!!.logoGravity = Gravity.END
        Assert.assertEquals(
            "Logo gravity should be same",
            Gravity.END.toLong(),
            uiSettings!!.logoGravity.toLong()
        )
    }

    @Test
    fun testLogoMargins() {
        Mockito.`when`(projection.contentPadding).thenReturn(intArrayOf(0, 0, 0, 0))
        Mockito.`when`(logoView.layoutParams).thenReturn(layoutParams)
        layoutParams.leftMargin = 1
        layoutParams.topMargin = 2
        layoutParams.rightMargin = 3
        layoutParams.bottomMargin = 4
        uiSettings!!.setLogoMargins(1, 2, 3, 4)
        Assert.assertTrue("Compass margin left should be same", uiSettings!!.logoMarginLeft == 1)
        Assert.assertTrue("Compass margin top should be same", uiSettings!!.logoMarginTop == 2)
        Assert.assertTrue("Compass margin right should be same", uiSettings!!.logoMarginRight == 3)
        Assert.assertTrue(
            "Compass margin bottom should be same",
            uiSettings!!.logoMarginBottom == 4
        )
    }

    @Test
    fun testAttributionEnabled() {
        uiSettings!!.isAttributionInitialized = true
        Mockito.`when`(attributionView.visibility).thenReturn(View.VISIBLE)
        uiSettings!!.isAttributionEnabled = true
        Assert.assertEquals(
            "Attribution should be enabled",
            true,
            uiSettings!!.isAttributionEnabled
        )
    }

    @Test
    fun testAttributionDisabled() {
        Mockito.`when`(attributionView.visibility).thenReturn(View.GONE)
        uiSettings!!.isAttributionEnabled = false
        Assert.assertEquals(
            "Attribution should be disabled",
            false,
            uiSettings!!.isAttributionEnabled
        )
    }

    @Test
    fun testAttributionGravity() {
        Mockito.`when`(attributionView.layoutParams).thenReturn(layoutParams)
        layoutParams.gravity = Gravity.END
        uiSettings!!.attributionGravity = Gravity.END
        Assert.assertEquals(
            "Attribution gravity should be same",
            Gravity.END.toLong(),
            uiSettings!!.attributionGravity.toLong()
        )
    }

    @Test
    fun testAttributionMargins() {
        Mockito.`when`(attributionView.layoutParams).thenReturn(layoutParams)
        Mockito.`when`(projection.contentPadding).thenReturn(intArrayOf(0, 0, 0, 0))
        layoutParams.leftMargin = 1
        layoutParams.topMargin = 2
        layoutParams.rightMargin = 3
        layoutParams.bottomMargin = 4
        uiSettings!!.setAttributionMargins(1, 2, 3, 4)
        Assert.assertTrue(
            "Attribution margin left should be same",
            uiSettings!!.attributionMarginLeft == 1
        )
        Assert.assertTrue(
            "Attribution margin top should be same",
            uiSettings!!.attributionMarginTop == 2
        )
        Assert.assertTrue(
            "Attribution margin right should be same",
            uiSettings!!.attributionMarginRight == 3
        )
        Assert.assertTrue(
            "Attribution margin bottom should be same",
            uiSettings!!.attributionMarginBottom == 4
        )
    }

    @Test
    fun testRotateGesturesEnabled() {
        uiSettings!!.isRotateGesturesEnabled = true
        Assert.assertEquals(
            "Rotate gesture should be enabled",
            true,
            uiSettings!!.isRotateGesturesEnabled
        )
    }

    @Test
    fun testRotateGesturesDisabled() {
        uiSettings!!.isRotateGesturesEnabled = false
        Assert.assertEquals(
            "Rotate gesture should be disabled",
            false,
            uiSettings!!.isRotateGesturesEnabled
        )
    }

    @Test
    fun testRotateGestureChangeAllowed() {
        uiSettings!!.isRotateGesturesEnabled = false
        Assert.assertEquals(
            "Rotate gesture should be false",
            false,
            uiSettings!!.isRotateGesturesEnabled
        )
        uiSettings!!.isRotateGesturesEnabled = true
        Assert.assertEquals(
            "Rotate gesture should be true",
            true,
            uiSettings!!.isRotateGesturesEnabled
        )
    }

    @Test
    fun testTiltGesturesEnabled() {
        uiSettings!!.isTiltGesturesEnabled = true
        Assert.assertEquals(
            "Tilt gesture should be enabled",
            true,
            uiSettings!!.isTiltGesturesEnabled
        )
    }

    @Test
    fun testTiltGesturesDisabled() {
        uiSettings!!.isTiltGesturesEnabled = false
        Assert.assertEquals(
            "Tilt gesture should be disabled",
            false,
            uiSettings!!.isTiltGesturesEnabled
        )
    }

    @Test
    fun testTiltGestureChangeAllowed() {
        uiSettings!!.isTiltGesturesEnabled = false
        Assert.assertEquals(
            "Tilt gesture should be false",
            false,
            uiSettings!!.isTiltGesturesEnabled
        )
        uiSettings!!.isTiltGesturesEnabled = true
        Assert.assertEquals("Tilt gesture should be true", true, uiSettings!!.isTiltGesturesEnabled)
    }

    @Test
    fun testZoomGesturesEnabled() {
        uiSettings!!.isZoomGesturesEnabled = true
        Assert.assertEquals(
            "Zoom gesture should be enabled",
            true,
            uiSettings!!.isZoomGesturesEnabled
        )
    }

    @Test
    fun testZoomGesturesDisabled() {
        uiSettings!!.isZoomGesturesEnabled = false
        Assert.assertEquals(
            "Zoom gesture should be disabled",
            false,
            uiSettings!!.isZoomGesturesEnabled
        )
    }

    @Test
    fun testZoomGestureChangeAllowed() {
        uiSettings!!.isZoomGesturesEnabled = false
        Assert.assertEquals(
            "Zoom gesture should be false",
            false,
            uiSettings!!.isZoomGesturesEnabled
        )
        uiSettings!!.isZoomGesturesEnabled = true
        Assert.assertEquals("Zoom gesture should be true", true, uiSettings!!.isZoomGesturesEnabled)
    }

    @Test
    fun testDoubleTapGesturesEnabled() {
        uiSettings!!.isDoubleTapGesturesEnabled = true
        Assert.assertEquals(
            "DoubleTap gesture should be enabled",
            true,
            uiSettings!!.isDoubleTapGesturesEnabled
        )
    }

    @Test
    fun testDoubleTapGesturesDisabled() {
        uiSettings!!.isDoubleTapGesturesEnabled = false
        Assert.assertEquals(
            "DoubleTap gesture should be disabled",
            false,
            uiSettings!!.isDoubleTapGesturesEnabled
        )
    }

    @Test
    fun testDoubleTapGestureChangeAllowed() {
        uiSettings!!.isDoubleTapGesturesEnabled = false
        Assert.assertEquals(
            "DoubleTap gesture should be false",
            false,
            uiSettings!!.isDoubleTapGesturesEnabled
        )
        uiSettings!!.isDoubleTapGesturesEnabled = true
        Assert.assertEquals(
            "DoubleTap gesture should be true",
            true,
            uiSettings!!.isDoubleTapGesturesEnabled
        )
    }

    @Test
    fun testQuickZoomGesturesEnabled() {
        uiSettings!!.isQuickZoomGesturesEnabled = true
        Assert.assertEquals(
            "QuickZoom gesture should be enabled",
            true,
            uiSettings!!.isQuickZoomGesturesEnabled
        )
    }

    @Test
    fun testQuickZoomGesturesDisabled() {
        uiSettings!!.isQuickZoomGesturesEnabled = false
        Assert.assertEquals(
            "QuickZoom gesture should be disabled",
            false,
            uiSettings!!.isQuickZoomGesturesEnabled
        )
    }

    @Test
    fun testQuickZoomGestureChangeAllowed() {
        uiSettings!!.isQuickZoomGesturesEnabled = false
        Assert.assertEquals(
            "QuickZoom gesture should be false",
            false,
            uiSettings!!.isQuickZoomGesturesEnabled
        )
        uiSettings!!.isQuickZoomGesturesEnabled = true
        Assert.assertEquals(
            "QuickZoom gesture should be true",
            true,
            uiSettings!!.isQuickZoomGesturesEnabled
        )
    }

    @Test
    fun testScrollGesturesEnabled() {
        uiSettings!!.isScrollGesturesEnabled = true
        Assert.assertEquals(
            "Scroll gesture should be enabled",
            true,
            uiSettings!!.isScrollGesturesEnabled
        )
    }

    @Test
    fun testScrollGesturesDisabled() {
        uiSettings!!.isScrollGesturesEnabled = false
        Assert.assertEquals(
            "Scroll gesture should be disabled",
            false,
            uiSettings!!.isScrollGesturesEnabled
        )
    }

    @Test
    fun testHorizontalScrollGesturesEnabled() {
        uiSettings!!.isHorizontalScrollGesturesEnabled = true
        Assert.assertEquals(
            "Scroll gesture should be enabled",
            true,
            uiSettings!!.isHorizontalScrollGesturesEnabled
        )
    }

    @Test
    fun testHorizontalScrollGesturesDisabled() {
        uiSettings!!.isHorizontalScrollGesturesEnabled = false
        Assert.assertEquals(
            "Scroll gesture should be disabled",
            false,
            uiSettings!!.isHorizontalScrollGesturesEnabled
        )
    }

    @Test
    fun testScrollGestureChangeAllowed() {
        uiSettings!!.isScrollGesturesEnabled = false
        Assert.assertEquals(
            "Scroll gesture should be false",
            false,
            uiSettings!!.isScrollGesturesEnabled
        )
        uiSettings!!.isScrollGesturesEnabled = true
        Assert.assertEquals(
            "Scroll gesture should be true",
            true,
            uiSettings!!.isScrollGesturesEnabled
        )
    }

    @Test
    fun testScaleVelocityAnimationEnabled() {
        uiSettings!!.isScaleVelocityAnimationEnabled = true
        Assert.assertEquals(
            "Scale velocity animation should be enabled",
            true,
            uiSettings!!.isScaleVelocityAnimationEnabled
        )
    }

    @Test
    fun testScaleVelocityAnimationDisabled() {
        uiSettings!!.isScaleVelocityAnimationEnabled = false
        Assert.assertEquals(
            "Scale velocity animation should be disabled",
            false,
            uiSettings!!.isScaleVelocityAnimationEnabled
        )
    }

    @Test
    fun testRotateVelocityAnimationEnabled() {
        uiSettings!!.isRotateVelocityAnimationEnabled = true
        Assert.assertEquals(
            "Rotate velocity animation should be enabled",
            true,
            uiSettings!!.isRotateVelocityAnimationEnabled
        )
    }

    @Test
    fun testRotateVelocityAnimationDisabled() {
        uiSettings!!.isRotateVelocityAnimationEnabled = false
        Assert.assertEquals(
            "Rotate velocity animation should be disabled",
            false,
            uiSettings!!.isRotateVelocityAnimationEnabled
        )
    }

    @Test
    fun testFlingVelocityAnimationEnabled() {
        uiSettings!!.isFlingVelocityAnimationEnabled = true
        Assert.assertEquals(
            "Fling velocity animation should be enabled",
            true,
            uiSettings!!.isFlingVelocityAnimationEnabled
        )
    }

    @Test
    fun testFlingVelocityAnimationDisabled() {
        uiSettings!!.isFlingVelocityAnimationEnabled = false
        Assert.assertEquals(
            "Fling velocity animation should be disabled",
            false,
            uiSettings!!.isFlingVelocityAnimationEnabled
        )
    }

    @Test
    fun testAllVelocityAnimationsEnabled() {
        uiSettings!!.setAllVelocityAnimationsEnabled(true)
        Assert.assertEquals(
            "Scale velocity animation should be enabled",
            true,
            uiSettings!!.isScaleVelocityAnimationEnabled
        )
        Assert.assertEquals(
            "Rotate velocity animation should be enabled",
            true,
            uiSettings!!.isRotateVelocityAnimationEnabled
        )
        Assert.assertEquals(
            "Fling velocity animation should be enabled",
            true,
            uiSettings!!.isFlingVelocityAnimationEnabled
        )
    }

    @Test
    fun testAllVelocityAnimationsDisabled() {
        uiSettings!!.setAllVelocityAnimationsEnabled(false)
        Assert.assertEquals(
            "Scale velocity animation should be disabled",
            false,
            uiSettings!!.isScaleVelocityAnimationEnabled
        )
        Assert.assertEquals(
            "Rotate velocity animation should be disabled",
            false,
            uiSettings!!.isRotateVelocityAnimationEnabled
        )
        Assert.assertEquals(
            "Fling velocity animation should be disabled",
            false,
            uiSettings!!.isFlingVelocityAnimationEnabled
        )
    }

    @Test
    fun testDisableRotateWhenScalingEnabled() {
        uiSettings!!.isDisableRotateWhenScaling = true
        Assert.assertEquals(
            "Rotate disabling should be enabled",
            true,
            uiSettings!!.isDisableRotateWhenScaling
        )
    }

    @Test
    fun testDisableRotateWhenScalingDisabled() {
        uiSettings!!.isDisableRotateWhenScaling = false
        Assert.assertEquals(
            "Rotate disabling should be disabled",
            false,
            uiSettings!!.isDisableRotateWhenScaling
        )
    }

    @Test
    fun testIncreaseScaleThresholdWhenRotatingEnabled() {
        uiSettings!!.isIncreaseScaleThresholdWhenRotating = true
        Assert.assertEquals(
            "Scale threshold increase should be enabled",
            true,
            uiSettings!!.isIncreaseScaleThresholdWhenRotating
        )
    }

    @Test
    fun testIncreaseScaleThresholdWhenRotatingDisabled() {
        uiSettings!!.isIncreaseScaleThresholdWhenRotating = false
        Assert.assertEquals(
            "Scale threshold increase should be disabled",
            false,
            uiSettings!!.isIncreaseScaleThresholdWhenRotating
        )
    }

    @Test
    fun testAllGesturesEnabled() {
        uiSettings!!.setAllGesturesEnabled(true)
        Assert.assertEquals(
            "Rotate gesture should be enabled",
            true,
            uiSettings!!.isRotateGesturesEnabled
        )
        Assert.assertEquals(
            "Tilt gesture should be enabled",
            true,
            uiSettings!!.isTiltGesturesEnabled
        )
        Assert.assertEquals(
            "Zoom gesture should be enabled",
            true,
            uiSettings!!.isZoomGesturesEnabled
        )
        Assert.assertEquals(
            "Scroll gesture should be enabled",
            true,
            uiSettings!!.isScrollGesturesEnabled
        )
    }

    @Test
    fun testAllGesturesDisabled() {
        uiSettings!!.setAllGesturesEnabled(false)
        Assert.assertEquals(
            "Rotate gesture should be enabled",
            false,
            uiSettings!!.isRotateGesturesEnabled
        )
        Assert.assertEquals(
            "Tilt gesture should be disabled",
            false,
            uiSettings!!.isTiltGesturesEnabled
        )
        Assert.assertEquals(
            "Zoom gesture should be disabled",
            false,
            uiSettings!!.isZoomGesturesEnabled
        )
        Assert.assertEquals(
            "Scroll gesture should be disabled",
            false,
            uiSettings!!.isScrollGesturesEnabled
        )
    }

    @Test
    fun testAreAllGesturesEnabled() {
        uiSettings!!.setAllGesturesEnabled(true)
        Assert.assertEquals(
            "All gestures check should return true",
            true,
            uiSettings!!.areAllGesturesEnabled()
        )
    }

    @Test
    fun testAreAllGesturesEnabledWithOneGestureDisabled() {
        uiSettings!!.setAllGesturesEnabled(true)
        uiSettings!!.isScrollGesturesEnabled = false
        Assert.assertEquals(
            "All gestures check should return false",
            false,
            uiSettings!!.areAllGesturesEnabled()
        )
    }

    @Test
    fun testZoomRateDefaultValue() {
        Assert.assertEquals(
            "Default zoom rate should be 1.0f",
            1.0f,
            uiSettings!!.zoomRate,
            0f
        )
    }

    @Test
    fun testZoomRate() {
        uiSettings!!.zoomRate = 0.83f
        Assert.assertEquals(
            "Zoom rate should be 0.83f",
            0.83f,
            uiSettings!!.zoomRate,
            0f
        )
    }

    @Test
    fun testUpdateWhenCompassViewNotHidden() {
        val cameraPosition = CameraPosition.Builder(CameraPosition.DEFAULT).bearing(24.0).build()
        Mockito.`when`(compassView.isHidden).thenReturn(false)
        uiSettings!!.update(cameraPosition)
        Mockito.verify(compassView).update(-24.0)
    }

    @Test
    fun testUpdateWhenCompassViewHidden() {
        val cameraPosition = CameraPosition.Builder(CameraPosition.DEFAULT).bearing(24.0).build()
        Mockito.`when`(compassView.isHidden).thenReturn(true)
        uiSettings!!.update(cameraPosition)
        Mockito.verify(compassView).update(-24.0)
    }
}
