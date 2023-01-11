package com.mapbox.mapboxsdk.testapp.activity

import androidx.annotation.UiThread
import com.mapbox.mapboxsdk.maps.MapboxMap
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.testapp.activity.espresso.EspressoTestActivity

/**
 * Base class for all tests using EspressoTestActivity as wrapper.
 *
 *
 * Loads "assets/streets.json" as style.
 *
 */
open class EspressoTest : BaseTest() {
    override fun getActivityClass(): Class<*> {
        return EspressoTestActivity::class.java
    }

    @UiThread
    override fun initMap(mapboxMap: MapboxMap) {
        mapboxMap.setStyle(Style.Builder().fromUri("asset://streets.json"))
        super.initMap(mapboxMap)
    }
}
