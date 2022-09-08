package com.mapbox.mapboxsdk.testapp.activity.fragment

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.FragmentManager
import com.mapbox.mapboxsdk.maps.MapboxMap
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.maps.SupportMapFragment
import com.mapbox.mapboxsdk.testapp.R

/**
 * Test Activity showcasing using multiple static map fragments in one layout.
 */
class MultiMapActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_multi_map)
        val fragmentManager = supportFragmentManager
        initFragmentStyle(fragmentManager, R.id.map1, Style.getPredefinedStyle("Streets"))
        initFragmentStyle(fragmentManager, R.id.map2, Style.getPredefinedStyle("Bright"))
        initFragmentStyle(fragmentManager, R.id.map3, Style.getPredefinedStyle("Satellite Hybrid"))
        initFragmentStyle(fragmentManager, R.id.map4, Style.getPredefinedStyle("Pastel"))
    }

    private fun initFragmentStyle(
        fragmentManager: FragmentManager,
        fragmentId: Int,
        styleId: String
    ) {
        (fragmentManager.findFragmentById(fragmentId) as SupportMapFragment?)
            ?.getMapAsync(OnMapReadyCallback { mapboxMap: MapboxMap -> mapboxMap.setStyle(styleId) })
    }
}
