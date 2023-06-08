// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.
package com.mapbox.mapboxsdk.location

package org.maplibre.android.location;

import android.graphics.Color;
import androidx.test.annotation.UiThreadTest;
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner;

import com.mapbox.geojson.LineString;
import com.mapbox.geojson.MultiLineString;
import com.mapbox.geojson.MultiPoint;
import com.mapbox.geojson.MultiPolygon;
import com.mapbox.geojson.Point;
import com.mapbox.geojson.Polygon;
import org.maplibre.android.maps.BaseLayerTest;
import org.junit.Before;
import timber.log.Timber;

import org.maplibre.android.style.expressions.Expression;

import org.junit.Test;
import org.junit.runner.RunWith;

import static org.maplibre.android.style.expressions.Expression.*;
import static org.junit.Assert.*;
import static org.maplibre.android.style.layers.Property.*;
import static org.maplibre.android.location.LocationPropertyFactory.*;

import org.maplibre.android.style.layers.TransitionOptions;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Basic smoke tests for LocationIndicatorLayer
 */
@RunWith(AndroidJUnit4ClassRunner::class)
class LocationIndicatorLayerTest : BaseLayerTest() {
    private var layer: LocationIndicatorLayer? = null

  private LocationIndicatorLayer layer;
  private final List<Point> pointsList = new ArrayList<Point>() {
    {
      add(Point.fromLngLat(55.30122473231012, 25.26476622289597));
      add(Point.fromLngLat(55.29743486255916, 25.25827212207261));
      add(Point.fromLngLat(55.28978863411328, 25.251356725509737));
      add(Point.fromLngLat(55.300027931336984, 25.246425506635504));
      add(Point.fromLngLat(55.307474692951274, 25.244200378933655));
      add(Point.fromLngLat(55.31212891895635, 25.256408010450187));
      add(Point.fromLngLat(55.30774064871093, 25.26266169122738));
      add(Point.fromLngLat(55.301357710197806, 25.264946609615492));
      add(Point.fromLngLat(55.30122473231012, 25.26476622289597));
    }
  };

    @Test
    @UiThreadTest
    fun testSetVisibility() {
        Timber.i("Visibility")
        Assert.assertNotNull(layer)

        // Get initial
        Assert.assertEquals(layer!!.visibility.getValue(), Property.VISIBLE)

        // Set
        layer!!.setProperties(LocationPropertyFactory.visibility(Property.NONE))
        Assert.assertEquals(layer!!.visibility.getValue(), Property.NONE)
    }

    @Test
    @UiThreadTest
    fun testTopImageAsConstant() {
        Timber.i("top-image")
        Assert.assertNotNull(layer)
        Assert.assertNull(layer!!.topImage.getValue())

        // Set and Get
        val propertyValue = "undefined"
        layer!!.setProperties(LocationPropertyFactory.topImage(propertyValue))
        Assert.assertEquals(layer!!.topImage.getValue(), propertyValue)
    }

    @Test
    @UiThreadTest
    fun testBearingImageAsConstant() {
        Timber.i("bearing-image")
        Assert.assertNotNull(layer)
        Assert.assertNull(layer!!.bearingImage.getValue())

        // Set and Get
        val propertyValue = "undefined"
        layer!!.setProperties(LocationPropertyFactory.bearingImage(propertyValue))
        Assert.assertEquals(layer!!.bearingImage.getValue(), propertyValue)
    }

    @Test
    @UiThreadTest
    fun testShadowImageAsConstant() {
        Timber.i("shadow-image")
        Assert.assertNotNull(layer)
        Assert.assertNull(layer!!.shadowImage.getValue())

        // Set and Get
        val propertyValue = "undefined"
        layer!!.setProperties(LocationPropertyFactory.shadowImage(propertyValue))
        Assert.assertEquals(layer!!.shadowImage.getValue(), propertyValue)
    }

    @Test
    @UiThreadTest
    fun testPerspectiveCompensationAsConstant() {
        Timber.i("perspective-compensation")
        Assert.assertNotNull(layer)
        Assert.assertNull(layer!!.perspectiveCompensation.getValue())

        // Set and Get
        val propertyValue = 0.3f
        layer!!.setProperties(LocationPropertyFactory.perspectiveCompensation(propertyValue))
        Assert.assertEquals(layer!!.perspectiveCompensation.getValue(), propertyValue)
    }

    @Test
    @UiThreadTest
    fun testImageTiltDisplacementAsConstant() {
        Timber.i("image-tilt-displacement")
        Assert.assertNotNull(layer)
        Assert.assertNull(layer!!.imageTiltDisplacement.getValue())

        // Set and Get
        val propertyValue = 0.3f
        layer!!.setProperties(LocationPropertyFactory.imageTiltDisplacement(propertyValue))
        Assert.assertEquals(layer!!.imageTiltDisplacement.getValue(), propertyValue)
    }

    @Test
    @UiThreadTest
    fun testBearingAsConstant() {
        Timber.i("bearing")
        Assert.assertNotNull(layer)
        Assert.assertNull(layer!!.bearing.getValue())

        // Set and Get
        val propertyValue = 0.3
        layer!!.setProperties(LocationPropertyFactory.bearing(propertyValue))
        Assert.assertEquals(layer!!.bearing.getValue(), propertyValue)
    }

    @Test
    @UiThreadTest
    fun testLocationTransition() {
        Timber.i("locationTransitionOptions")
        Assert.assertNotNull(layer)

        // Set and Get
        val options = TransitionOptions(300, 100)
        layer!!.locationTransition = options
        Assert.assertEquals(layer!!.locationTransition, options)
    }

    @Test
    @UiThreadTest
    fun testLocationAsConstant() {
        Timber.i("location")
        Assert.assertNotNull(layer)
        Assert.assertNull(layer!!.location.getValue())

        // Set and Get
        val propertyValue = arrayOf(0.0, 0.0, 0.0)
        layer!!.setProperties(LocationPropertyFactory.location(propertyValue))
        Assert.assertEquals(layer!!.location.getValue(), propertyValue)
    }

    @Test
    @UiThreadTest
    fun testAccuracyRadiusTransition() {
        Timber.i("accuracy-radiusTransitionOptions")
        Assert.assertNotNull(layer)

        // Set and Get
        val options = TransitionOptions(300, 100)
        layer!!.accuracyRadiusTransition = options
        Assert.assertEquals(layer!!.accuracyRadiusTransition, options)
    }

    @Test
    @UiThreadTest
    fun testAccuracyRadiusAsConstant() {
        Timber.i("accuracy-radius")
        Assert.assertNotNull(layer)
        Assert.assertNull(layer!!.accuracyRadius.getValue())

        // Set and Get
        val propertyValue = 0.3f
        layer!!.setProperties(LocationPropertyFactory.accuracyRadius(propertyValue))
        Assert.assertEquals(layer!!.accuracyRadius.getValue(), propertyValue)
    }

    @Test
    @UiThreadTest
    fun testTopImageSizeTransition() {
        Timber.i("top-image-sizeTransitionOptions")
        Assert.assertNotNull(layer)

        // Set and Get
        val options = TransitionOptions(300, 100)
        layer!!.topImageSizeTransition = options
        Assert.assertEquals(layer!!.topImageSizeTransition, options)
    }

    @Test
    @UiThreadTest
    fun testTopImageSizeAsConstant() {
        Timber.i("top-image-size")
        Assert.assertNotNull(layer)
        Assert.assertNull(layer!!.topImageSize.getValue())

        // Set and Get
        val propertyValue = 0.3f
        layer!!.setProperties(LocationPropertyFactory.topImageSize(propertyValue))
        Assert.assertEquals(layer!!.topImageSize.getValue(), propertyValue)
    }

    @Test
    @UiThreadTest
    fun testBearingImageSizeTransition() {
        Timber.i("bearing-image-sizeTransitionOptions")
        Assert.assertNotNull(layer)

        // Set and Get
        val options = TransitionOptions(300, 100)
        layer!!.bearingImageSizeTransition = options
        Assert.assertEquals(layer!!.bearingImageSizeTransition, options)
    }

    @Test
    @UiThreadTest
    fun testBearingImageSizeAsConstant() {
        Timber.i("bearing-image-size")
        Assert.assertNotNull(layer)
        Assert.assertNull(layer!!.bearingImageSize.getValue())

        // Set and Get
        val propertyValue = 0.3f
        layer!!.setProperties(LocationPropertyFactory.bearingImageSize(propertyValue))
        Assert.assertEquals(layer!!.bearingImageSize.getValue(), propertyValue)
    }

    @Test
    @UiThreadTest
    fun testShadowImageSizeTransition() {
        Timber.i("shadow-image-sizeTransitionOptions")
        Assert.assertNotNull(layer)

        // Set and Get
        val options = TransitionOptions(300, 100)
        layer!!.shadowImageSizeTransition = options
        Assert.assertEquals(layer!!.shadowImageSizeTransition, options)
    }

    @Test
    @UiThreadTest
    fun testShadowImageSizeAsConstant() {
        Timber.i("shadow-image-size")
        Assert.assertNotNull(layer)
        Assert.assertNull(layer!!.shadowImageSize.getValue())

        // Set and Get
        val propertyValue = 0.3f
        layer!!.setProperties(LocationPropertyFactory.shadowImageSize(propertyValue))
        Assert.assertEquals(layer!!.shadowImageSize.getValue(), propertyValue)
    }

    @Test
    @UiThreadTest
    fun testAccuracyRadiusColorTransition() {
        Timber.i("accuracy-radius-colorTransitionOptions")
        Assert.assertNotNull(layer)

        // Set and Get
        val options = TransitionOptions(300, 100)
        layer!!.accuracyRadiusColorTransition = options
        Assert.assertEquals(layer!!.accuracyRadiusColorTransition, options)
    }

    @Test
    @UiThreadTest
    fun testAccuracyRadiusColorAsConstant() {
        Timber.i("accuracy-radius-color")
        Assert.assertNotNull(layer)
        Assert.assertNull(layer!!.accuracyRadiusColor.getValue())

        // Set and Get
        val propertyValue = "rgba(255,128,0,0.7)"
        layer!!.setProperties(LocationPropertyFactory.accuracyRadiusColor(propertyValue))
        Assert.assertEquals(layer!!.accuracyRadiusColor.getValue(), propertyValue)
    }

    @Test
    @UiThreadTest
    fun testAccuracyRadiusColorAsIntConstant() {
        Timber.i("accuracy-radius-color")
        Assert.assertNotNull(layer)

        // Set and Get
        layer!!.setProperties(
            LocationPropertyFactory.accuracyRadiusColor(
                Color.argb(
                    127,
                    255,
                    127,
                    0
                )
            )
        )
        Assert.assertEquals(
            layer!!.accuracyRadiusColorAsInt.toLong(),
            Color.argb(127, 255, 127, 0).toLong()
        )
    }

    @Test
    @UiThreadTest
    fun testAccuracyRadiusBorderColorTransition() {
        Timber.i("accuracy-radius-border-colorTransitionOptions")
        Assert.assertNotNull(layer)

        // Set and Get
        val options = TransitionOptions(300, 100)
        layer!!.accuracyRadiusBorderColorTransition = options
        Assert.assertEquals(layer!!.accuracyRadiusBorderColorTransition, options)
    }

    @Test
    @UiThreadTest
    fun testAccuracyRadiusBorderColorAsConstant() {
        Timber.i("accuracy-radius-border-color")
        Assert.assertNotNull(layer)
        Assert.assertNull(layer!!.accuracyRadiusBorderColor.getValue())

        // Set and Get
        val propertyValue = "rgba(255,128,0,0.7)"
        layer!!.setProperties(LocationPropertyFactory.accuracyRadiusBorderColor(propertyValue))
        Assert.assertEquals(layer!!.accuracyRadiusBorderColor.getValue(), propertyValue)
    }

    @Test
    @UiThreadTest
    fun testAccuracyRadiusBorderColorAsIntConstant() {
        Timber.i("accuracy-radius-border-color")
        Assert.assertNotNull(layer)

        // Set and Get
        layer!!.setProperties(
            LocationPropertyFactory.accuracyRadiusBorderColor(
                Color.argb(
                    127,
                    255,
                    127,
                    0
                )
            )
        )
        Assert.assertEquals(
            layer!!.accuracyRadiusBorderColorAsInt.toLong(),
            Color.argb(127, 255, 127, 0).toLong()
        )
    }
}
