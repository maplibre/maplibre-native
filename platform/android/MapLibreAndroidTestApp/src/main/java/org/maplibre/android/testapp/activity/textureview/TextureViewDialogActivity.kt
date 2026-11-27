package org.maplibre.android.testapp.activity.textureview

import android.app.Dialog
import android.os.Bundle
import android.view.ViewGroup
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.maps.MapLibreMapOptions
import org.maplibre.android.maps.MapView
import org.maplibre.android.testapp.styles.TestStyles

/**
 * Test displaying a [android.view.TextureView] backed map in a dialog.
 */
class TextureViewDialogActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private lateinit var dialog: Dialog

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        mapView = MapView(this, MapLibreMapOptions.createFromAttributes(this).textureMode(true))
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync {
            it.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets"))
        }

        dialog = Dialog(this)
        dialog.setContentView(mapView)
        dialog.setOnCancelListener { finish() }
        dialog.show()
        dialog.window?.setLayout(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.MATCH_PARENT,
        )
    }

    override fun onStart() {
        super.onStart()
        mapView.onStart()
    }

    override fun onResume() {
        super.onResume()
        mapView.onResume()
    }

    override fun onPause() {
        super.onPause()
        mapView.onPause()
    }

    override fun onStop() {
        super.onStop()
        mapView.onStop()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    override fun onDestroy() {
        dialog.dismiss()
        mapView.onDestroy()
        super.onDestroy()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }
}
