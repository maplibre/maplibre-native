package org.maplibre.android.testapp.utils

import androidx.test.espresso.IdlingResource
import org.maplibre.android.maps.MapView

/// Connect the MapView's idle state to Espresso's IdlingResource, so that tests can wait for the map to finish.
class MapIdleResource(private val mapView: MapView) : IdlingResource {
    private val listener = MapView.OnDidBecomeIdleListener {
        if (!isIdle) {
            callback?.onTransitionToIdle()
        }
        isIdle = true
    }

    init {
        mapView.addOnDidBecomeIdleListener(listener)
    }

    /// Remove the view listener
    fun unregister() {
        mapView.removeOnDidBecomeIdleListener(listener)
    }

    override fun getName(): String {
        return javaClass.simpleName
    }

    // There's no event for the map becoming non-idle, so we have to reset
    // the idle state manually when we know the map is no longer idle.
    fun reset() {
        isIdle = false
    }

    override fun isIdleNow(): Boolean {
        return isIdle
    }

    override fun registerIdleTransitionCallback(callback: IdlingResource.ResourceCallback?) {
        this.callback = callback
    }

    private var isIdle = false
    private var callback: IdlingResource.ResourceCallback? = null
}
