package org.maplibre.android.testapp.activity

import androidx.annotation.UiThread
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Style
import org.maplibre.android.testapp.activity.espresso.EspressoTestActivity
import org.maplibre.android.testapp.styles.TestStyles

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
    override fun initMap(maplibreMap: MapLibreMap) {
        maplibreMap.setStyle(Style.Builder().fromUri(TestStyles.VERSATILES))
        super.initMap(maplibreMap)
    }
}
