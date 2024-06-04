package org.maplibre.android.testapp.activity.maplayout

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.*
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.DialogFragment
import org.maplibre.android.maps.*
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles

/**
 * Test activity showcasing showing a Map inside of a DialogFragment.
 */
class MapInDialogActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_map_in_dialog)
        val button = findViewById<Button>(R.id.button_open_dialog)
        button.setOnClickListener { view: View? ->
            val fm = supportFragmentManager
            val editNameDialogFragment = MapDialogFragment.newInstance("Map Dialog")
            editNameDialogFragment.show(fm, "fragment_dialog_map")
        }
    }

    class MapDialogFragment : DialogFragment() {
        private lateinit var mapView: MapView
        override fun onCreateView(
            inflater: LayoutInflater,
            container: ViewGroup?,
            savedInstanceState: Bundle?
        ): View? {
            return inflater.inflate(R.layout.fragment_dialog_map, container)
        }

        override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
            super.onViewCreated(view, savedInstanceState)
            mapView = view.findViewById(R.id.mapView)
            mapView.onCreate(savedInstanceState)
            mapView.getMapAsync(
                OnMapReadyCallback { maplibreMap: MapLibreMap ->
                    maplibreMap.setStyle(
                        TestStyles.getPredefinedStyleWithFallback("Outdoor")
                    )
                }
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

        override fun onDestroyView() {
            super.onDestroyView()
            mapView.onDestroy()
        }

        override fun onLowMemory() {
            super.onLowMemory()
            if (mapView != null) {
                mapView.onLowMemory()
            }
        }

        override fun onSaveInstanceState(outState: Bundle) {
            super.onSaveInstanceState(outState)
            if (mapView != null) {
                mapView.onSaveInstanceState(outState)
            }
        }

        companion object {
            fun newInstance(title: String?): MapDialogFragment {
                val frag = MapDialogFragment()
                val args = Bundle()
                args.putString("title", title)
                frag.arguments = args
                return frag
            }
        }
    }
}
