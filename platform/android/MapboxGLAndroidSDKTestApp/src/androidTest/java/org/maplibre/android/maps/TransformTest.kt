package org.maplibre.android.maps

import androidx.test.espresso.UiController
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.testapp.action.MapboxMapAction.invoke
import org.maplibre.android.testapp.activity.EspressoTest
import org.junit.Assert.assertEquals
import org.junit.Test

class TransformTest : EspressoTest() {

    companion object {
        val initialCameraUpdate = CameraUpdateFactory.newLatLngZoom(LatLng(12.0, 12.0), 12.0)!!
    }

    @Test
    fun mapboxMapScrollByWithPadding() {
        validateTestSetup()
        invoke(mapboxMap) { _: UiController, mapboxMap: MapboxMap ->
            mapboxMap.moveCamera(initialCameraUpdate)
            mapboxMap.scrollBy(400.0f, 0.0f)
            val expectedCameraPosition = mapboxMap.cameraPosition

            mapboxMap.moveCamera(initialCameraUpdate)
            mapboxMap.setPadding(250, 250, 0, 0)
            mapboxMap.scrollBy(400.0f, 0.0f)
            val actualCameraPosition = mapboxMap.cameraPosition

            assertEquals("Camera position should match", expectedCameraPosition, actualCameraPosition)
        }
    }
}
