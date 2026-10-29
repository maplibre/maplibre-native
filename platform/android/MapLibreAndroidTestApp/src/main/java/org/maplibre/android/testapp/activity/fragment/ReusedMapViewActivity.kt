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
import timber.log.Timber

/**
 * Test activity for a single activity app that reuses one SurfaceView based MapView for all of
 * its screens.
 *
 * The MapView is created with the activity, which forwards its lifecycle to it. The map screen
 * adds the MapView to its layout. Opening another screen takes the MapView out of it right away,
 * so the MapView is detached from the window whenever another screen is shown and attached again
 * when the map screen comes back, while the activity stays resumed. Attaching and detaching is
 * logged with the tag ReusedMapViewActivity.
 */
class ReusedMapViewActivity : AppCompatActivity() {

    companion object {
        private const val TAG = "ReusedMapViewActivity"
    }

    lateinit var mapView: MapView
        private set

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(FrameLayout(this).apply { id = R.id.container })

        // A SurfaceView, unlike a TextureView, stops its render thread whenever it is detached
        mapView = MapView(this, MapLibreMapOptions.createFromAttributes(this).textureMode(false))
        mapView.id = R.id.mapView
        mapView.addOnAttachStateChangeListener(object : View.OnAttachStateChangeListener {
            override fun onViewAttachedToWindow(view: View) {
                Timber.tag(TAG).i("MapView attached to the window")
            }

            override fun onViewDetachedFromWindow(view: View) {
                Timber.tag(TAG).i("MapView detached from the window")
            }
        })
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
        // Detach the MapView right away instead of whenever the fragment manager tears down the map
        // screen's view, like an app that moves the MapView out of the screen it leaves
        detachMapView()
        supportFragmentManager.beginTransaction()
            .replace(R.id.container, ScreenWithoutMapFragment())
            .addToBackStack(null)
            .commit()
    }

    /** Takes the MapView out of the screen showing it, which detaches it from the window. */
    private fun detachMapView() {
        (mapView.parent as? ViewGroup)?.removeView(mapView)
        check(!mapView.isAttachedToWindow) { "The MapView is still attached to the window" }
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

        private val hostActivity: ReusedMapViewActivity
            get() = requireActivity() as ReusedMapViewActivity

        override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View {
            val root = FrameLayout(inflater.context)
            val mapView = hostActivity.mapView
            hostActivity.detachMapView()
            root.addView(mapView, FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT)

            val openButton = Button(inflater.context)
            openButton.setText(R.string.reused_map_view_open_screen_without_map)
            openButton.setOnClickListener { hostActivity.openScreenWithoutMap() }
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
            // The MapView outlives this screen, take it out of the view that is being destroyed. This
            // also covers leaving the screen without openScreenWithoutMap, e.g. when the activity ends.
            hostActivity.detachMapView()
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
