package org.maplibre.android.annotations

import android.graphics.PointF
import androidx.annotation.UiThread
import androidx.annotation.VisibleForTesting
import androidx.collection.LongSparseArray
import com.mapbox.geojson.Feature
import com.mapbox.geojson.FeatureCollection
import java.util.concurrent.atomic.AtomicBoolean
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.log.Logger
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapLibreMap.OnMapClickListener
import org.maplibre.android.maps.MapLibreMap.OnMapLongClickListener
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.Layer
import org.maplibre.android.style.layers.PropertyValue
import org.maplibre.android.style.sources.GeoJsonOptions
import org.maplibre.android.style.sources.GeoJsonSource

/**
 * Generic AnnotationManager, can be used to create annotation specific managers.
 *
 * @param T type of annotation
 * @param S type of options for building the annotation, depends on generic T
 * @param D type of annotation drag listener, depends on generic T
 * @param U type of annotation click listener, depends on generic T
 * @param V type of annotation long click listener, depends on generic T
 */
abstract class AnnotationManager<L : Layer, T : AbstractAnnotation<*>, S : Options<T>, D : OnAnnotationDragListener<T>, U : OnAnnotationClickListener<T>, V : OnAnnotationLongClickListener<T>> @UiThread internal constructor(
    private val mapView: MapView,
    protected val maplibreMap: MapLibreMap,
    private var style: Style,
    protected var coreElementProvider: CoreElementProvider<L>,
    private val draggableAnnotationController: DraggableAnnotationController,
    private val belowLayerId: String?,
    private val aboveLayerId: String?,
    geoJsonOptions: GeoJsonOptions?
) {
    /**
     * Get a list of current annotations.
     *
     * @return long sparse array of annotations
     */
    @get:UiThread
    val annotations = LongSparseArray<T>()
    val dataDrivenPropertyUsageMap: MutableMap<String, Boolean> = HashMap()
    val constantPropertyUsageMap: MutableMap<String, PropertyValue<*>> = HashMap()
    var layerFilter: Expression? = null
    private val dragListeners: MutableList<D> = ArrayList()
    private val clickListeners: MutableList<U> = ArrayList()
    private val longClickListeners: MutableList<V> = ArrayList()
    private var currentId: Long = 0
    protected lateinit var layer: L
    protected lateinit var geoJsonSource: GeoJsonSource
    private val mapClickResolver: MapClickResolver
    private val isSourceUpToDate = AtomicBoolean(true)

    init {
        if (!style.isFullyLoaded) {
            throw RuntimeException("The style has to be non-null and fully loaded.")
        }
        mapClickResolver = MapClickResolver().also {
            maplibreMap.addOnMapClickListener(it)
            maplibreMap.addOnMapLongClickListener(it)
        }
        draggableAnnotationController.addAnnotationManager(this)
        initializeSourcesAndLayers(geoJsonOptions)
        mapView.addOnDidFinishLoadingStyleListener {
            maplibreMap.getStyle { loadedStyle ->
                style = loadedStyle
                initializeSourcesAndLayers(geoJsonOptions)
            }
        }
    }

    val layerId: String
        /**
         * Returns a layer ID that annotations created by this manager are laid out on.
         *
         *
         * This reference can be used together with [Style.addLayerAbove]
         * or [Style.addLayerBelow] to improve other layers positioning in relation to this manager.
         *
         * @return underlying layer's ID
         */
        get() = layer.id

    /**
     * Create an annotation on the map
     *
     * @param options the annotation options defining the annotation to build
     * @return the built annotation
     */
    @UiThread
    fun create(options: S): T = options.build(currentId, this).also {
        annotations.put(it.id, it)
        currentId++
        updateSource()
    }

    /**
     * Create a list of annotations on the map.
     *
     * @param optionsList the list of annotation options defining the list of annotations to build
     * @return the list of built annotations
     */
    @UiThread
    fun create(optionsList: List<S>) = optionsList.map { create(it) }

    /**
     * Delete an annotation from the map.
     *
     * @param annotation annotation to be deleted
     */
    @UiThread
    fun delete(annotation: T) {
        annotations.remove(annotation.id)
        draggableAnnotationController.onAnnotationDeleted(annotation)
        updateSource()
    }

    /**
     * Deletes annotations from the map.
     *
     * @param annotationList the list of annotations to be deleted
     */
    @UiThread
    fun delete(annotationList: List<T>) {
        annotationList.forEach {
            annotations.remove(it.id)
            draggableAnnotationController.onAnnotationDeleted(it)
        }
        updateSource()
    }

    /**
     * Deletes all annotations from the map.
     */
    @UiThread
    fun deleteAll() {
        annotations.clear()
        updateSource()
    }

    /**
     * Update an annotation on the map.
     *
     * @param annotation annotation to be updated
     */
    @UiThread
    fun update(annotation: T) {
        if (annotations.containsValue(annotation)) {
            annotations.put(annotation.id, annotation)
            updateSource()
        } else {
            Logger.e(
                TAG,
                "Can't update annotation: $annotation, the annotation isn't active annotation."
            )
        }
    }

    /**
     * Update annotations on the map.
     *
     * @param annotationList list of annotation to be updated
     */
    @UiThread
    fun update(annotationList: List<T>) {
        annotationList.forEach {
            annotations.put(it.id, it)
        }
        updateSource()
    }

    /**
     * Trigger an update to the underlying source. The update is delayed until after
     * the next UI draw to batch multiple actions.
     */
    fun updateSource() {
        // Only schedule a new refresh if not already scheduled
        if (isSourceUpToDate.compareAndSet(true, false)) {
            mapView.post {
                isSourceUpToDate.set(true)
                if (style.isFullyLoaded) {
                    updateSourceNow()
                } else {
                    // We are in progress of loading a new style
                    return@post
                }
            }
        }
    }

    /**
     * Undelayed source update, only used for testing and by [updateSource].
     */
    @VisibleForTesting
    internal fun updateSourceNow() {
        val features: MutableList<Feature> = ArrayList()
        for (i in 0 until annotations.size()) {
            annotations.valueAt(i).let {
                features.add(Feature.fromGeometry(it.geometry, it.feature))
                it.setUsedDataDrivenProperties()
            }
        }
        geoJsonSource.setGeoJson(FeatureCollection.fromFeatures(features))
    }

    fun enableDataDrivenProperty(property: String) {
        if ((dataDrivenPropertyUsageMap[property] == false)) {
            dataDrivenPropertyUsageMap[property] = true
            setDataDrivenPropertyIsUsed(property)
        }
    }

    protected abstract fun setDataDrivenPropertyIsUsed(property: String)

    /**
     * Add a callback to be invoked when an annotation is dragged.
     *
     * @param d the callback to be invoked when an annotation is dragged
     */
    @UiThread
    fun addDragListener(d: D) {
        dragListeners.add(d)
    }

    /**
     * Remove a previously added callback that was to be invoked when an annotation has been dragged.
     *
     * @param d the callback to be removed
     */
    @UiThread
    fun removeDragListener(d: D) {
        dragListeners.remove(d)
    }

    /**
     * Add a callback to be invoked when a symbol has been clicked.
     *
     * @param u the callback to be invoked when a symbol is clicked
     */
    @UiThread
    fun addClickListener(u: U) {
        clickListeners.add(u)
    }

    /**
     * Remove a previously added callback that was to be invoked when symbol has been clicked.
     *
     * @param u the callback to be removed
     */
    @UiThread
    fun removeClickListener(u: U) {
        clickListeners.remove(u)
    }

    /**
     * Add a callback to be invoked when a symbol has been long clicked.
     *
     * @param v the callback to be invoked when a symbol is clicked
     */
    @UiThread
    fun addLongClickListener(v: V) {
        longClickListeners.add(v)
    }

    /**
     * Remove a previously added callback that was to be invoked when symbol has been long clicked.
     *
     * @param v the callback to be removed
     */
    @UiThread
    fun removeLongClickListener(v: V) {
        longClickListeners.remove(v)
    }

    @VisibleForTesting
    fun getClickListeners(): List<U> {
        return clickListeners
    }

    @VisibleForTesting
    fun getLongClickListeners(): List<V> {
        return longClickListeners
    }

    fun getDragListeners(): List<D> {
        return dragListeners
    }

    /**
     * Cleanup annotation manager, used to clear listeners
     */
    @UiThread
    fun onDestroy() {
        maplibreMap.removeOnMapClickListener(mapClickResolver)
        maplibreMap.removeOnMapLongClickListener(mapClickResolver)
        draggableAnnotationController.removeAnnotationManager(this)
        dragListeners.clear()
        clickListeners.clear()
        longClickListeners.clear()
    }

    abstract val annotationIdKey: String?

    abstract fun initializeDataDrivenPropertyMap()

    abstract fun setFilter(expression: Expression)

    private fun initializeSourcesAndLayers(geoJsonOptions: GeoJsonOptions?) {
        geoJsonSource = coreElementProvider.getSource(geoJsonOptions).also {
            style.addSource(it)
        }

        if (belowLayerId != null && aboveLayerId != null) {
            throw IllegalArgumentException(
                "At most one of belowLayerId and aboveLayerId can be set, not both!"
            )
        }

        layer = coreElementProvider.layer.also {
            if (belowLayerId != null) {
                style.addLayerBelow(it, belowLayerId)
            } else if (aboveLayerId != null) {
                style.addLayerAbove(it, aboveLayerId)
            } else {
                style.addLayer(it)
            }
        }

        initializeDataDrivenPropertyMap()
        layer.setProperties(*constantPropertyUsageMap.values.toTypedArray())
        layerFilter?.let {
            setFilter(it)
        }

        updateSource()
    }

    /**
     * Inner class for transforming map click events into annotation clicks
     */
    private inner class MapClickResolver : OnMapClickListener, OnMapLongClickListener {

        override fun onMapClick(point: LatLng): Boolean =
            if (clickListeners.isNotEmpty()) {
                queryMapForFeatures(point)?.let { annotation ->
                    clickListeners.any { it.onAnnotationClick(annotation) }
                } ?: false
            } else {
                false
            }

        override fun onMapLongClick(point: LatLng): Boolean =
            if (longClickListeners.isNotEmpty()) {
                queryMapForFeatures(point)?.let { annotation ->
                    longClickListeners.any { it.onAnnotationLongClick(annotation) }
                } ?: false
            } else {
                false
            }
    }

    private fun queryMapForFeatures(point: LatLng): T? =
        queryMapForFeatures(maplibreMap.projection.toScreenLocation(point))

    fun queryMapForFeatures(point: PointF): T? =
        maplibreMap.queryRenderedFeatures(point, coreElementProvider.layerId).firstOrNull()
            ?.let { feature ->
                val id = feature.getProperty(annotationIdKey).asLong
                annotations[id]
            }

    companion object {
        private const val TAG = "AnnotationManager"
    }
}
