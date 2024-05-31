package org.maplibre.android.testapp.activity.textureview

import org.maplibre.android.maps.MapLibreMapOptions
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.testapp.activity.maplayout.DebugModeActivity
import org.maplibre.android.testapp.utils.NavUtils

/**
 * Test activity showcasing the different debug modes and allows to cycle between the default map styles.
 */
class TextureViewDebugModeActivity : DebugModeActivity(), OnMapReadyCallback {
    override fun setupMapLibreMapOptions(): MapLibreMapOptions {
        val maplibreMapOptions = super.setupMapLibreMapOptions()
        maplibreMapOptions.textureMode(true)
        return maplibreMapOptions
    }

    override fun onBackPressed() {
        // activity uses singleInstance for testing purposes
        // code below provides a default navigation when using the app
        NavUtils.navigateHome(this)
    }
}
