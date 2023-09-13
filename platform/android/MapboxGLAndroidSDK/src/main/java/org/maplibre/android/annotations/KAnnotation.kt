package org.maplibre.android.annotations

import com.google.gson.JsonElement
import com.mapbox.android.gestures.MoveDistancesObject
import com.mapbox.geojson.Geometry
import org.maplibre.android.annotations.data.Defaults
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Projection
import timber.log.Timber

sealed class KAnnotation<T : Geometry>(
    zLayer: Int = Defaults.Z_LAYER,
    draggable: Boolean = Defaults.DRAGGABLE,
    data: JsonElement? = Defaults.JSON_ELEMENT,
    var clickListener: OnAnnotationClickListener<out KAnnotation<T>>? = null,
    var dragListener: OnAnnotationDragListener<out KAnnotation<T>>? = null,
    var longClickListener: OnAnnotationLongClickListener<out KAnnotation<T>>? = null
) {

    var zLayer = zLayer
        set(value) {
            field = value
            updateAll()
        }

    @get:JvmName("isDraggable")
    var draggable = draggable
        set(value) {
            field = value
            updateThis()
        }

    var data = data
        set(value) {
            field = value
            updateThis()
        }

    private var attachedToMap: MapLibreMap? = null
    private var attachedToManager: AnnotationManager<*, *>? = null

    internal var id: Long = 0
        private set

    internal abstract var geometry: T
    internal abstract val dataDrivenProperties: List<Triple<String, Any?, Any?>>

    internal open fun updateThis() {
        attachedToMap?.updateAnnotation(this)
        attachedToManager?.updateSource()
    }

    internal fun updateAll() {
        attachedToMap?.updateAnnotations()
        attachedToManager?.updateSource()?.let {
            Timber.w("You have changed a property that has a different, or no, effect on Annotations that " +
                    "are added to an AnnotationManager. If you don't know what you are doing, we advise you to add " +
                    "the Annotation to a MapLibreMap instead; this will ensure that everything behaves as you " +
                    "would expect it to.")
        }
    }

    @JvmName("attach")
    internal fun attach(to: MapLibreMap, id: Long) {
        // If added to a MaplibreMap, the annotation manager that this annotation belongs to may
        // change frequently. Therefore we assign MapLibreMap (by extension its container class) the
        // responsibility of knowing which annotation manager is responsible.
        if (attachedToMap == null && attachedToManager == null && this.id == 0L) {
            attachedToMap = to
            this.id = id
        } else {
            throw IllegalStateException(
                "An Annotation can only be added to either MapLibreMap or AnnotationManager, and it cannot be added " +
                    "a second time. This annotation was previously added to " +
                    "${if (attachedToMap == null) "a MaplibreMap" else "an AnnotationManager"}, and can therefore " +
                    "not be added to a MaplibreMap now."
            )
        }
    }

    @JvmName("attach")
    internal fun attach(to: AnnotationManager<*, *>, id: Long) {
        if (attachedToMap == null && attachedToManager == null && this.id == 0L) {
            attachedToManager = to
            this.id = id
        } else {
            throw IllegalStateException(
                "An Annotation can only be added to either MapLibreMap or AnnotationManager, and it cannot be added " +
                    "a second time. This annotation was previously added to " +
                    "${if (attachedToMap == null) "a MapLibreMap" else "an AnnotationManager"}, and can therefore " +
                    "not be added to an AnnotationManager now."
            )
        }
    }

    abstract fun getOffsetGeometry(
        projection: Projection,
        moveDistancesObject: MoveDistancesObject,
        touchAreaShiftX: Float,
        touchAreaShiftY: Float
    ): Geometry?
}
