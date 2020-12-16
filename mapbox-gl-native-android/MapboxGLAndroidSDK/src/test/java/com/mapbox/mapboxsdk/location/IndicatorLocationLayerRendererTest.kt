package com.mapbox.mapboxsdk.location

import android.graphics.Bitmap
import android.graphics.Color
import com.mapbox.mapboxsdk.geometry.LatLng
import com.mapbox.mapboxsdk.location.LocationComponentConstants.*
import com.mapbox.mapboxsdk.location.modes.RenderMode
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.style.expressions.Expression
import com.mapbox.mapboxsdk.style.layers.Layer
import com.mapbox.mapboxsdk.style.layers.Property
import com.mapbox.mapboxsdk.utils.BitmapUtils
import com.mapbox.mapboxsdk.utils.ColorUtils
import io.mockk.*
import org.junit.Assert
import org.junit.Before
import org.junit.Test

class IndicatorLocationLayerRendererTest {

  private val style: Style = mockk(relaxUnitFun = true)
  private val layerSourceProvider: LayerSourceProvider = mockk(relaxUnitFun = true)
  private val layer: Layer = mockk(relaxUnitFun = true)

  private lateinit var locationLayerRenderer: IndicatorLocationLayerRenderer

  @Before
  fun setup() {
    every { style.removeLayer(any<Layer>()) } returns true
    locationLayerRenderer = IndicatorLocationLayerRenderer(layerSourceProvider)
    every { layerSourceProvider.generateLocationComponentLayer() } returns layer
    locationLayerRenderer.initializeComponents(style)
  }

  @Test
  fun sanity() {
    Assert.assertNotNull(locationLayerRenderer)
  }

  @Test
  fun initializeComponents_withLocation() {
    val newLayer: Layer = mockk(relaxUnitFun = true)
    every { layerSourceProvider.generateLocationComponentLayer() } returns newLayer
    val latLng = LatLng(10.0, 20.0)
    val bearing = 11f
    val accuracy = 65f
    locationLayerRenderer.setLatLng(latLng)
    locationLayerRenderer.setGpsBearing(bearing)
    locationLayerRenderer.setAccuracyRadius(accuracy)

    locationLayerRenderer.initializeComponents(mockk())

    verify { newLayer.setProperties(LocationPropertyFactory.location(latLng.toLocationArray())) }
    verify { newLayer.setProperties(LocationPropertyFactory.bearing(bearing.toDouble())) }
    verify { newLayer.setProperties(LocationPropertyFactory.accuracyRadius(accuracy)) }
  }

  @Test
  fun addLayers() {
    val positionManager: LocationComponentPositionManager = mockk(relaxUnitFun = true)

    locationLayerRenderer.addLayers(positionManager)

    verify { positionManager.addLayerToMap(layer) }
  }

  @Test
  fun removeLayers() {
    locationLayerRenderer.removeLayers()

    verify { style.removeLayer(layer) }
  }

  @Test
  fun hide() {
    locationLayerRenderer.hide()

    verify { layer.setProperties(LocationPropertyFactory.visibility(Property.NONE)) }
  }

  @Test
  fun show() {
    locationLayerRenderer.show(RenderMode.NORMAL, false)

    verify { layer.setProperties(LocationPropertyFactory.visibility(Property.VISIBLE)) }
  }

  @Test
  fun show_normal_notStale() {
    locationLayerRenderer.show(RenderMode.NORMAL, false)

    verify {
      layer.setProperties(
        LocationPropertyFactory.topImage(FOREGROUND_ICON),
        LocationPropertyFactory.bearingImage(BACKGROUND_ICON),
        LocationPropertyFactory.shadowImage(SHADOW_ICON)
      )
    }
  }

  @Test
  fun show_compass_notStale() {
    locationLayerRenderer.show(RenderMode.COMPASS, false)

    verify {
      layer.setProperties(
        LocationPropertyFactory.topImage(FOREGROUND_ICON),
        LocationPropertyFactory.bearingImage(BEARING_ICON),
        LocationPropertyFactory.shadowImage(SHADOW_ICON)
      )
    }
  }

  @Test
  fun show_gps_notStale() {
    locationLayerRenderer.show(RenderMode.GPS, false)

    verify {
      layer.setProperties(
        LocationPropertyFactory.topImage(""),
        LocationPropertyFactory.bearingImage(FOREGROUND_ICON),
        LocationPropertyFactory.shadowImage(BACKGROUND_ICON)
      )
    }
    verify { layer.setProperties(LocationPropertyFactory.accuracyRadius(0f)) }
  }

  @Test
  fun show_normal_stale() {
    locationLayerRenderer.show(RenderMode.NORMAL, true)

    verify {
      layer.setProperties(
        LocationPropertyFactory.topImage(FOREGROUND_STALE_ICON),
        LocationPropertyFactory.bearingImage(BACKGROUND_STALE_ICON),
        LocationPropertyFactory.shadowImage(SHADOW_ICON)
      )
    }
  }

  @Test
  fun show_compass_stale() {
    locationLayerRenderer.show(RenderMode.COMPASS, true)

    verify {
      layer.setProperties(
        LocationPropertyFactory.topImage(FOREGROUND_STALE_ICON),
        LocationPropertyFactory.bearingImage(BEARING_STALE_ICON),
        LocationPropertyFactory.shadowImage(SHADOW_ICON)
      )
    }
  }

  @Test
  fun show_gps_stale() {
    locationLayerRenderer.show(RenderMode.GPS, true)

    verify {
      layer.setProperties(
        LocationPropertyFactory.topImage(""),
        LocationPropertyFactory.bearingImage(FOREGROUND_STALE_ICON),
        LocationPropertyFactory.shadowImage(BACKGROUND_STALE_ICON)
      )
    }
    verify { layer.setProperties(LocationPropertyFactory.accuracyRadius(0f)) }
  }

  @Test
  fun styleAccuracy() {
    val colorArray = ColorUtils.colorToRgbaArray(Color.RED)
    val exp = Expression.rgba(colorArray[0], colorArray[1], colorArray[2], 0.7f)

    locationLayerRenderer.styleAccuracy(0.7f, Color.RED)

    verify {
      layer.setProperties(
        LocationPropertyFactory.accuracyRadiusColor(exp),
        LocationPropertyFactory.accuracyRadiusBorderColor(exp)
      )
    }
  }

  @Test
  fun setLatLng() {
    val latLng = LatLng(10.0, 20.0)
    locationLayerRenderer.setLatLng(latLng)

    verify { layer.setProperties(LocationPropertyFactory.location(latLng.toLocationArray())) }
  }

  @Test
  fun setGpsBearing() {
    val bearing = 30.0
    locationLayerRenderer.setGpsBearing(bearing.toFloat())

    verify { layer.setProperties(LocationPropertyFactory.bearing(bearing)) }
  }

  @Test
  fun setCompassBearing() {
    val bearing = 30.0
    locationLayerRenderer.setCompassBearing(bearing.toFloat())

    verify { layer.setProperties(LocationPropertyFactory.bearing(bearing)) }
  }

  @Test
  fun setAccuracyRadius() {
    val radius = 40f
    locationLayerRenderer.setAccuracyRadius(radius)

    verify { layer.setProperties(LocationPropertyFactory.accuracyRadius(radius)) }
  }

  @Test
  fun styleScaling() {
    val exp = Expression.literal("")
    locationLayerRenderer.styleScaling(exp)

    verify {
      layer.setProperties(
        LocationPropertyFactory.shadowImageSize(exp),
        LocationPropertyFactory.bearingImageSize(exp),
        LocationPropertyFactory.topImageSize(exp)
      )
    }
  }

  @Test
  fun setLocationStale() {
    locationLayerRenderer.setLocationStale(true, RenderMode.NORMAL)

    verify {
      layer.setProperties(
        LocationPropertyFactory.topImage(FOREGROUND_STALE_ICON),
        LocationPropertyFactory.bearingImage(BACKGROUND_STALE_ICON),
        LocationPropertyFactory.shadowImage(SHADOW_ICON)
      )
    }
  }

  @Test
  fun addBitmaps_shadow() {
    addBitmaps(withShadow = true, renderMode = RenderMode.NORMAL)

    verify { style.addImage(SHADOW_ICON, shadowBitmap) }
  }

  @Test
  fun addBitmaps_noShadow() {
    addBitmaps(withShadow = false, renderMode = RenderMode.NORMAL)

    verify { style.removeImage(SHADOW_ICON) }
  }

  @Test
  fun addBitmaps_normal() {
    addBitmaps(withShadow = true, renderMode = RenderMode.NORMAL)

    verify { style.addImage(FOREGROUND_ICON, foregroundBitmap) }
    verify { style.addImage(FOREGROUND_STALE_ICON, foregroundStaleBitmap) }
    verify { style.addImage(BACKGROUND_ICON, backgroundBitmap) }
    verify { style.addImage(BACKGROUND_STALE_ICON, backgroundStaleBitmap) }
    verify { style.addImage(BEARING_ICON, bearingBitmap) }
  }

  @Test
  fun addBitmaps_compass() {
    every { bearingBitmap.width } returns 40
    every { bearingBitmap.height } returns 40
    every { bearingBitmap.config } returns mockk()
    every { backgroundBitmap.width } returns 20
    every { backgroundBitmap.height } returns 10

    val mergedBitmap = mockk<Bitmap>()
    mockkStatic(BitmapUtils::class)
    every { BitmapUtils.mergeBitmap(bearingBitmap, backgroundBitmap, 10f, 15f) } returns mergedBitmap

    every { bearingBitmap.width } returns 40
    every { bearingBitmap.height } returns 40
    every { bearingBitmap.config } returns mockk()
    every { backgroundStaleBitmap.width } returns 30
    every { backgroundStaleBitmap.height } returns 10

    val mergedStaleBitmap = mockk<Bitmap>()
    every { BitmapUtils.mergeBitmap(bearingBitmap, backgroundStaleBitmap, 5f, 15f) } returns mergedStaleBitmap

    addBitmaps(withShadow = true, renderMode = RenderMode.COMPASS)

    verify { style.addImage(BEARING_ICON, mergedBitmap) }
    verify { style.addImage(BEARING_STALE_ICON, mergedStaleBitmap) }

    unmockkStatic(BitmapUtils::class)
  }

  @Test
  fun addBitmaps_gps() {
    addBitmaps(withShadow = true, renderMode = RenderMode.GPS)

    verify { style.addImage(FOREGROUND_ICON, foregroundBitmap) }
    verify { style.addImage(FOREGROUND_STALE_ICON, foregroundStaleBitmap) }
    verify { style.addImage(BACKGROUND_ICON, backgroundBitmap) }
    verify { style.addImage(BACKGROUND_STALE_ICON, backgroundStaleBitmap) }
    verify { style.addImage(BEARING_ICON, bearingBitmap) }
  }

  private val shadowBitmap = mockk<Bitmap>()
  private val backgroundBitmap = mockk<Bitmap>()
  private val backgroundStaleBitmap = mockk<Bitmap>()
  private val bearingBitmap = mockk<Bitmap>()
  private val foregroundBitmap = mockk<Bitmap>()
  private val foregroundStaleBitmap = mockk<Bitmap>()

  private fun addBitmaps(withShadow: Boolean, @RenderMode.Mode renderMode: Int) {
    locationLayerRenderer.addBitmaps(
      renderMode,
      if (withShadow) shadowBitmap else null,
      backgroundBitmap,
      backgroundStaleBitmap,
      bearingBitmap,
      foregroundBitmap,
      foregroundStaleBitmap
    )
  }

  private fun LatLng.toLocationArray() = arrayOf(latitude, longitude, 0.0)
}