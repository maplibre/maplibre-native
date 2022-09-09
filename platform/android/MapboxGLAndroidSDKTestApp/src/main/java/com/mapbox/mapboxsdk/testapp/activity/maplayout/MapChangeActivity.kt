package com.mapbox.mapboxsdk.testapp.activity.maplayout

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.mapbox.mapboxsdk.camera.CameraUpdateFactory
import com.mapbox.mapboxsdk.geometry.LatLng
import com.mapbox.mapboxsdk.maps.*
import com.mapbox.mapboxsdk.maps.MapView.OnCameraDidChangeListener
import com.mapbox.mapboxsdk.maps.MapView.OnCameraIsChangingListener
import com.mapbox.mapboxsdk.maps.MapView.OnCameraWillChangeListener
import com.mapbox.mapboxsdk.maps.MapView.OnDidBecomeIdleListener
import com.mapbox.mapboxsdk.maps.MapView.OnDidFailLoadingMapListener
import com.mapbox.mapboxsdk.maps.MapView.OnDidFinishLoadingMapListener
import com.mapbox.mapboxsdk.maps.MapView.OnDidFinishLoadingStyleListener
import com.mapbox.mapboxsdk.maps.MapView.OnDidFinishRenderingFrameListener
import com.mapbox.mapboxsdk.maps.MapView.OnDidFinishRenderingMapListener
import com.mapbox.mapboxsdk.maps.MapView.OnSourceChangedListener
import com.mapbox.mapboxsdk.maps.MapView.OnWillStartLoadingMapListener
import com.mapbox.mapboxsdk.maps.MapView.OnWillStartRenderingFrameListener
import com.mapbox.mapboxsdk.maps.MapView.OnWillStartRenderingMapListener
import com.mapbox.mapboxsdk.testapp.R
import timber.log.Timber

/**
 * Test activity showcasing how to listen to map change events.
 */
class MapChangeActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_map_simple)
        mapView = findViewById(R.id.mapView)
        mapView.addOnCameraIsChangingListener(OnCameraIsChangingListener { Timber.v("OnCameraIsChanging") })
        mapView.addOnCameraDidChangeListener(
            OnCameraDidChangeListener { animated: Boolean ->
                Timber.v(
                    "OnCamaraDidChange: animated: %s",
                    animated
                )
            }
        )
        mapView.addOnCameraWillChangeListener(
            OnCameraWillChangeListener { animated: Boolean ->
                Timber.v(
                    "OnCameraWilChange: animated: %s",
                    animated
                )
            }
        )
        mapView.addOnDidFailLoadingMapListener(
            OnDidFailLoadingMapListener { errorMessage: String? ->
                Timber.v(
                    "OnDidFailLoadingMap: %s",
                    errorMessage
                )
            }
        )
        mapView.addOnDidFinishLoadingMapListener(OnDidFinishLoadingMapListener { Timber.v("OnDidFinishLoadingMap") })
        mapView.addOnDidFinishLoadingStyleListener(OnDidFinishLoadingStyleListener { Timber.v("OnDidFinishLoadingStyle") })
        mapView.addOnDidFinishRenderingFrameListener(
            OnDidFinishRenderingFrameListener { fully: Boolean ->
                Timber.v(
                    "OnDidFinishRenderingFrame: fully: %s",
                    fully
                )
            }
        )
        mapView.addOnDidFinishRenderingMapListener(
            OnDidFinishRenderingMapListener { fully: Boolean ->
                Timber.v(
                    "OnDidFinishRenderingMap: fully: %s",
                    fully
                )
            }
        )
        mapView.addOnDidBecomeIdleListener(OnDidBecomeIdleListener { Timber.v("OnDidBecomeIdle") })
        mapView.addOnSourceChangedListener(
            OnSourceChangedListener { sourceId: String? ->
                Timber.v(
                    "OnSourceChangedListener: source with id: %s",
                    sourceId
                )
            }
        )
        mapView.addOnWillStartLoadingMapListener(OnWillStartLoadingMapListener { Timber.v("OnWillStartLoadingMap") })
        mapView.addOnWillStartRenderingFrameListener(OnWillStartRenderingFrameListener { Timber.v("OnWillStartRenderingFrame") })
        mapView.addOnWillStartRenderingMapListener(OnWillStartRenderingMapListener { Timber.v("OnWillStartRenderingMap") })
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(
            OnMapReadyCallback { mapboxMap: MapboxMap ->
                mapboxMap.setStyle(Style.getPredefinedStyle("Streets"))
                mapboxMap.animateCamera(
                    CameraUpdateFactory.newLatLngZoom(
                        LatLng(55.754020, 37.620948),
                        12.0
                    ),
                    9000
                )
            }
        )
    }

    override fun onStart() {
        super.onStart()
        mapView!!.onStart()
    }

    override fun onResume() {
        super.onResume()
        mapView!!.onResume()
    }

    override fun onPause() {
        super.onPause()
        mapView!!.onPause()
    }

    override fun onStop() {
        super.onStop()
        mapView!!.onStop()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView!!.onLowMemory()
    }

    override fun onDestroy() {
        super.onDestroy()
        mapView!!.onDestroy()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView!!.onSaveInstanceState(outState)
    }
}
