package org.maplibre.android.testapp.activity.fragment

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Style
import org.maplibre.android.maps.SupportMapFragment
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.databinding.ActivityBackstackFragmentBinding
import org.maplibre.android.testapp.utils.NavUtils

/**
 * Test activity showcasing using the MapFragment API as part of a backstacked fragment.
 */
class FragmentBackStackActivity : AppCompatActivity() {

    companion object {
        private const val FRAGMENT_TAG = "map_fragment"
    }

    private lateinit var binding: ActivityBackstackFragmentBinding
    private lateinit var mapFragment: SupportMapFragment

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityBackstackFragmentBinding.inflate(layoutInflater)
        setContentView(binding.root)

        if (savedInstanceState == null) {
            mapFragment = SupportMapFragment.newInstance()
            mapFragment.getMapAsync { initMap(it) }

            supportFragmentManager.beginTransaction().apply {
                add(R.id.container, mapFragment, FRAGMENT_TAG)
            }.commit()
        } else {
            supportFragmentManager.findFragmentByTag(FRAGMENT_TAG)?.also { fragment ->
                if (fragment is SupportMapFragment) {
                    fragment.getMapAsync { initMap(it) }
                }
            }
        }

        binding.button.setOnClickListener { handleClick() }
    }

    private fun initMap(maplibreMap: MapLibreMap) {
        maplibreMap.setStyle(Style.getPredefinedStyle("Satellite Hybrid")) {
            maplibreMap.setPadding(300, 300, 300, 300)
        }
    }

    private fun handleClick() {
        supportFragmentManager.beginTransaction().apply {
            replace(R.id.container, NestedViewPagerActivity.ItemAdapter.EmptyFragment())
            addToBackStack("map_empty_fragment")
        }.commit()
    }

    override fun onBackPressed() {
        if (supportFragmentManager.backStackEntryCount == 0) {
            // activity uses singleInstance for testing purposes
            // code below provides a default navigation when using the app
            NavUtils.navigateHome(this)
        } else {
            super.onBackPressed()
        }
    }
}
