package org.maplibre.android.maps

import android.content.Context
import android.os.Bundle
import android.util.AttributeSet
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import org.maplibre.android.utils.MapFragmentUtils

/**
 * Fragment wrapper around a map view.
 *
 * A Map component in an app. This fragment is the simplest way to place a map in an application.
 * It's a wrapper around a view of a map to automatically handle the necessary life cycle needs.
 * Being a fragment, this component can be added to an activity's layout or can dynamically be added
 * using a FragmentManager.
 *
 * To get a reference to the MapView, use [getMapAsync]
 *
 * @see getMapAsync
 */
class MapFragment :
    Fragment(),
    OnMapReadyCallback {
    private val mapReadyCallbackList = mutableListOf<OnMapReadyCallback>()
    private var mapViewReadyCallback: OnMapViewReadyCallback? = null
    private var maplibreMap: MapLibreMap? = null
    private var map: MapView? = null

    /**
     * Called when this fragment is inflated, parses XML tag attributes.
     *
     * @param context            The context inflating this fragment.
     * @param attrs              The XML tag attributes.
     * @param savedInstanceState The saved instance state for the map fragment.
     */
    override fun onInflate(
        context: Context,
        attrs: AttributeSet,
        savedInstanceState: Bundle?,
    ) {
        super.onInflate(context, attrs, savedInstanceState)
        arguments = MapFragmentUtils.createFragmentArgs(MapLibreMapOptions.createFromAttributes(context, attrs))
    }

    /**
     * Called when the context attaches to this fragment.
     *
     * @param context the context attaching
     */
    override fun onAttach(context: Context) {
        super.onAttach(context)
        if (context is OnMapViewReadyCallback) {
            mapViewReadyCallback = context
        }
    }

    /**
     * Creates the fragment view hierarchy.
     *
     * @param inflater           Inflater used to inflate content.
     * @param container          The parent layout for the map fragment.
     * @param savedInstanceState The saved instance state for the map fragment.
     * @return The view created
     */
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?,
    ): View {
        super.onCreateView(inflater, container, savedInstanceState)
        val context = inflater.context
        val mapView = MapView(context, MapFragmentUtils.resolveArgs(context, arguments))
        map = mapView
        return mapView
    }

    /**
     * Called when the fragment view hierarchy is created.
     *
     * @param view               The content view of the fragment
     * @param savedInstanceState The saved instance state of the fragment
     */
    override fun onViewCreated(
        view: View,
        savedInstanceState: Bundle?,
    ) {
        super.onViewCreated(view, savedInstanceState)
        val mapView = map ?: return
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(this)

        // notify listeners about mapview creation
        mapViewReadyCallback?.onMapViewReady(mapView)
    }

    /**
     * Called when the style of the map has successfully loaded.
     *
     * @param maplibreMap The public api controller of the map
     */
    override fun onMapReady(maplibreMap: MapLibreMap) {
        this.maplibreMap = maplibreMap
        for (onMapReadyCallback in mapReadyCallbackList) {
            onMapReadyCallback.onMapReady(maplibreMap)
        }
    }

    /**
     * Called when the fragment is visible for the users.
     */
    override fun onStart() {
        super.onStart()
        map?.onStart()
    }

    /**
     * Called when the fragment is ready to be interacted with.
     */
    override fun onResume() {
        super.onResume()
        map?.onResume()
    }

    /**
     * Called when the fragment is pausing.
     */
    override fun onPause() {
        super.onPause()
        map?.onPause()
    }

    /**
     * Called when the fragment state needs to be saved.
     *
     * @param outState The saved state
     */
    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        map?.takeIf { !it.isDestroyed }?.onSaveInstanceState(outState)
    }

    /**
     * Called when the fragment is no longer visible for the user.
     */
    override fun onStop() {
        super.onStop()
        map?.onStop()
    }

    /**
     * Called when the fragment receives onLowMemory call from the hosting Activity.
     */
    override fun onLowMemory() {
        super.onLowMemory()
        map?.takeIf { !it.isDestroyed }?.onLowMemory()
    }

    /**
     * Called when the fragment is view hiearchy is being destroyed.
     */
    override fun onDestroyView() {
        super.onDestroyView()
        map?.onDestroy()
    }

    /**
     * Called when the fragment is destroyed.
     */
    override fun onDestroy() {
        super.onDestroy()
        mapReadyCallbackList.clear()
    }

    /**
     * Sets a callback object which will be triggered when the MapLibreMap instance is ready to be used.
     *
     * @param onMapReadyCallback The callback to be invoked.
     */
    fun getMapAsync(onMapReadyCallback: OnMapReadyCallback) {
        val map = maplibreMap
        if (map == null) {
            mapReadyCallbackList.add(onMapReadyCallback)
        } else {
            onMapReadyCallback.onMapReady(map)
        }
    }

    /**
     * Callback to be invoked when the map fragment has inflated its MapView.
     *
     * To use this interface the context hosting the fragment must implement this interface.
     * That instance will be set as part of Fragment#onAttach(Context context).
     */
    fun interface OnMapViewReadyCallback {
        /**
         * Called when the map has been created.
         *
         * @param mapView The created mapview
         */
        fun onMapViewReady(mapView: MapView)
    }

    companion object {
        /**
         * Creates a default MapFragment instance
         *
         * @return MapFragment instantiated
         */
        @JvmStatic
        fun newInstance(): MapFragment = MapFragment()

        /**
         * Creates a MapFragment instance
         *
         * @param maplibreMapOptions The configuration options to be used.
         * @return MapFragment instantiated.
         */
        @JvmStatic
        fun newInstance(maplibreMapOptions: MapLibreMapOptions?): MapFragment {
            val mapFragment = MapFragment()
            mapFragment.arguments = MapFragmentUtils.createFragmentArgs(maplibreMapOptions)
            return mapFragment
        }
    }
}
