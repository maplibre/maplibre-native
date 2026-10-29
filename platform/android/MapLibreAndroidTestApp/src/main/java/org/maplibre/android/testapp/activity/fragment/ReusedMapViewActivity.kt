package org.maplibre.android.testapp.activity.fragment

import android.os.Bundle
import android.view.Gravity
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.FrameLayout
import android.widget.LinearLayout
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.Fragment
import org.maplibre.android.maps.MapLibreMapOptions
import org.maplibre.android.maps.MapView
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles

/**
 * Test activity for a single activity app that reuses one SurfaceView based MapView for all of
 * its screens.
 *
 * The MapView is created with the activity, which forwards its lifecycle to it. The map screen
 * adds the MapView to its layout and removes it again when its view is destroyed, so the MapView
 * is detached from the window whenever another screen is shown and attached again when the map
 * screen comes back, while the activity stays resumed.
 */
class ReusedMapViewActivity : AppCompatActivity() {

    lateinit var mapView: MapView
        private set

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(FrameLayout(this).apply { id = R.id.container })

        // A SurfaceView, unlike a TextureView, stops its render thread whenever it is detached
        mapView = MapView(this, MapLibreMapOptions.createFromAttributes(this).textureMode(false))
        mapView.id = R.id.mapView
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync { it.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets")) }

        if (savedInstanceState == null) {
            supportFragmentManager.beginTransaction()
                .add(R.id.container, MapScreenFragment())
                .commitNow()
        }
    }

    /** Replaces the map screen with a screen without a map, which detaches the MapView. */
    fun openScreenWithoutMap() {
        supportFragmentManager.beginTransaction()
            .replace(R.id.container, ScreenWithoutMapFragment())
            .addToBackStack(null)
            .commit()
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
        super.onDestroy()
        mapView.onDestroy()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    /** Shows the activity's MapView for as long as its view exists. */
    class MapScreenFragment : Fragment() {

        private val mapView: MapView
            get() = (requireActivity() as ReusedMapViewActivity).mapView

        override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View {
            val root = FrameLayout(inflater.context)
            root.addView(mapView, FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT)

            val openButton = Button(inflater.context)
            openButton.setText(R.string.reused_map_view_open_screen_without_map)
            openButton.setOnClickListener { (requireActivity() as ReusedMapViewActivity).openScreenWithoutMap() }
            root.addView(
                openButton,
                FrameLayout.LayoutParams(
                    FrameLayout.LayoutParams.MATCH_PARENT,
                    FrameLayout.LayoutParams.WRAP_CONTENT,
                    Gravity.BOTTOM
                )
            )
            return root
        }

        override fun onDestroyView() {
            // The MapView outlives this screen, take it out of the view that is being destroyed
            (requireView() as ViewGroup).removeView(mapView)
            super.onDestroyView()
        }
    }

    class ScreenWithoutMapFragment : Fragment() {

        override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View {
            val root = LinearLayout(inflater.context)
            root.orientation = LinearLayout.VERTICAL
            root.gravity = Gravity.CENTER

            val text = TextView(inflater.context)
            text.setText(R.string.reused_map_view_screen_without_map)
            root.addView(text)

            val backButton = Button(inflater.context)
            backButton.setText(R.string.reused_map_view_back_to_map)
            backButton.setOnClickListener { parentFragmentManager.popBackStack() }
            root.addView(backButton)
            return root
        }
    }
}
