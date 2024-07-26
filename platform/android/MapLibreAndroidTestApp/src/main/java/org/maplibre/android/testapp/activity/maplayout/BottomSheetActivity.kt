package org.maplibre.android.testapp.activity.maplayout

import android.content.Context
import android.os.Bundle
import android.view.*
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.Fragment
import com.google.android.material.bottomsheet.BottomSheetBehavior
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.*
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import org.maplibre.android.utils.MapFragmentUtils

/**
 * Test activity showcasing using a bottomView with a MapView and stacking map fragments below.
 */
class BottomSheetActivity : AppCompatActivity() {
    private var bottomSheetFragmentAdded = false
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_bottom_sheet)
        val actionBar = supportActionBar
        actionBar?.setDisplayHomeAsUpEnabled(true)
        findViewById<View>(R.id.fabFragment).setOnClickListener { v: View? -> addMapFragment() }
        findViewById<View>(R.id.fabBottomSheet).setOnClickListener { v: View? -> toggleBottomSheetMapFragment() }
        val bottomSheetBehavior: BottomSheetBehavior<*> =
            BottomSheetBehavior.from(findViewById<View>(R.id.bottom_sheet))
        bottomSheetBehavior.peekHeight = (64 * resources.displayMetrics.density).toInt()
        bottomSheetBehavior.state = BottomSheetBehavior.STATE_COLLAPSED
        toggleBottomSheetMapFragment()
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        if (item.itemId == android.R.id.home) {
            onBackPressed()
        }
        return super.onOptionsItemSelected(item)
    }

    override fun onBackPressed() {
        val fragmentManager = supportFragmentManager
        if (fragmentManager.backStackEntryCount > 0) {
            fragmentManager.popBackStack()
        } else {
            super.onBackPressed()
        }
    }

    private fun addMapFragment() {
        val fragmentManager = supportFragmentManager
        val fragmentCount = fragmentManager.backStackEntryCount
        val fragmentTransaction = fragmentManager.beginTransaction()
        val mainMapFragment = MainMapFragment.newInstance(this, fragmentCount)
        if (fragmentCount == 0) {
            fragmentTransaction.add(R.id.fragment_container, mainMapFragment, TAG_MAIN_FRAGMENT)
        } else {
            fragmentTransaction.replace(R.id.fragment_container, mainMapFragment, TAG_MAIN_FRAGMENT)
        }
        fragmentTransaction.addToBackStack(mainMapFragment.hashCode().toString())
        fragmentTransaction.commit()
        Toast.makeText(
            applicationContext,
            String.format(AMOUNT_OF_MAIN_MAP_FRAGMENTS, fragmentCount + 1),
            Toast.LENGTH_SHORT
        ).show()
    }

    private fun toggleBottomSheetMapFragment() {
        if (!bottomSheetFragmentAdded) {
            addBottomSheetMapFragment()
        } else {
            removeBottomSheetFragment()
        }
        bottomSheetFragmentAdded = !bottomSheetFragmentAdded
    }

    private fun addBottomSheetMapFragment() {
        val fragmentTransaction = supportFragmentManager.beginTransaction()
        fragmentTransaction.add(
            R.id.fragment_container_bottom,
            BottomSheetFragment.newInstance(this),
            TAG_BOTTOM_FRAGMENT
        )
        fragmentTransaction.commit()
    }

    private fun removeBottomSheetFragment() {
        val fragment = supportFragmentManager.findFragmentByTag(TAG_BOTTOM_FRAGMENT)
        if (fragment != null) {
            supportFragmentManager.beginTransaction().remove(fragment).commit()
        }
    }

    class MainMapFragment : Fragment(), OnMapReadyCallback {
        private var map: MapView? = null
        override fun onCreateView(
            inflater: LayoutInflater,
            container: ViewGroup?,
            savedInstanceState: Bundle?
        ): View? {
            super.onCreateView(inflater, container, savedInstanceState)
            val context = inflater.context
            return MapView(context, MapFragmentUtils.resolveArgs(context, arguments)).also {
                map = it
            }
        }

        override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
            super.onViewCreated(view, savedInstanceState)
            map!!.onCreate(savedInstanceState)
            map!!.getMapAsync(this)
        }

        override fun onMapReady(maplibreMap: MapLibreMap) {
            maplibreMap.moveCamera(
                CameraUpdateFactory.newLatLngZoom(
                    LatLng(37.760545, -122.436055),
                    15.0
                )
            )
            maplibreMap.setStyle(
                Style.Builder().fromUri(
                    STYLES[
                        Math.min(
                            Math.max(arguments!!.getInt("mapcounter"), 0),
                            STYLES.size - 1
                        )
                    ]
                )
            )
        }

        override fun onStart() {
            super.onStart()
            map!!.onStart()
        }

        override fun onResume() {
            super.onResume()
            map!!.onResume()
        }

        override fun onPause() {
            super.onPause()
            map!!.onPause()
        }

        override fun onStop() {
            super.onStop()
            map!!.onStop()
        }

        override fun onSaveInstanceState(outState: Bundle) {
            super.onSaveInstanceState(outState)
            map!!.onSaveInstanceState(outState)
        }

        override fun onLowMemory() {
            super.onLowMemory()
            map!!.onLowMemory()
        }

        override fun onDestroyView() {
            super.onDestroyView()
            map!!.onDestroy()
        }

        companion object {
            private val STYLES = arrayOf(
                TestStyles.getPredefinedStyleWithFallback("Streets"),
                TestStyles.getPredefinedStyleWithFallback("Satellite Hybrid"),
                TestStyles.getPredefinedStyleWithFallback("Bright"),
                TestStyles.getPredefinedStyleWithFallback("Pastel"),
                TestStyles.getPredefinedStyleWithFallback("Satellite Hybrid"),
                TestStyles.getPredefinedStyleWithFallback("Outdoor")
            )

            fun newInstance(context: Context?, mapCounter: Int): MainMapFragment {
                val mapFragment = MainMapFragment()
                val bundle = Bundle()
                bundle.putInt("mapcounter", mapCounter)
                mapFragment.arguments = bundle
                val maplibreMapOptions = MapLibreMapOptions.createFromAttributes(context!!)
                mapFragment.arguments = MapFragmentUtils.createFragmentArgs(maplibreMapOptions)
                return mapFragment
            }
        }
    }

    class BottomSheetFragment : Fragment(), OnMapReadyCallback {
        private var map: MapView? = null
        override fun onCreateView(
            inflater: LayoutInflater,
            container: ViewGroup?,
            savedInstanceState: Bundle?
        ): View? {
            super.onCreateView(inflater, container, savedInstanceState)
            val context = inflater.context
            return MapView(context, MapFragmentUtils.resolveArgs(context, arguments)).also {
                map = it
            }
        }

        override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
            super.onViewCreated(view, savedInstanceState)
            map!!.onCreate(savedInstanceState)
            map!!.getMapAsync(this)
        }

        override fun onMapReady(maplibreMap: MapLibreMap) {
            maplibreMap.moveCamera(
                CameraUpdateFactory.newLatLngZoom(
                    LatLng(37.760545, -122.436055),
                    15.0
                )
            )
            maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Bright"))
        }

        override fun onStart() {
            super.onStart()
            map!!.onStart()
        }

        override fun onResume() {
            super.onResume()
            map!!.onResume()
        }

        override fun onPause() {
            super.onPause()
            map!!.onPause()
        }

        override fun onStop() {
            super.onStop()
            map!!.onStop()
        }

        override fun onSaveInstanceState(outState: Bundle) {
            super.onSaveInstanceState(outState)
            map!!.onSaveInstanceState(outState)
        }

        override fun onLowMemory() {
            super.onLowMemory()
            map!!.onLowMemory()
        }

        override fun onDestroyView() {
            super.onDestroyView()
            map!!.onDestroy()
        }

        companion object {
            fun newInstance(context: Context?): BottomSheetFragment {
                val mapFragment = BottomSheetFragment()
                val maplibreMapOptions = MapLibreMapOptions.createFromAttributes(context!!)
                maplibreMapOptions.renderSurfaceOnTop(true)
                mapFragment.arguments = MapFragmentUtils.createFragmentArgs(maplibreMapOptions)
                return mapFragment
            }
        }
    }

    companion object {
        private const val TAG_MAIN_FRAGMENT = "com.mapbox.mapboxsdk.fragment.tag.main"
        private const val TAG_BOTTOM_FRAGMENT = "com.mapbox.mapboxsdk.fragment.tag.bottom"
        private const val AMOUNT_OF_MAIN_MAP_FRAGMENTS = "Amount of main map fragments: %s"
    }
}
