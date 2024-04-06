package org.maplibre.android.annotations

import android.graphics.Bitmap
import androidx.annotation.ColorInt
import com.mapbox.android.gestures.MoveDistancesObject
import org.maplibre.android.annotations.data.Defaults
import org.maplibre.android.annotations.data.Translate
import org.maplibre.android.constants.GeometryConstants.MAX_MERCATOR_LATITUDE
import org.maplibre.android.constants.GeometryConstants.MIN_MERCATOR_LATITUDE
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.Projection
import org.maplibre.geojson.Geometry
import org.maplibre.geojson.Point
import org.maplibre.geojson.Polygon

class Fill @JvmOverloads constructor(
    paths: List<List<LatLng>>,
    opacity: Float = Defaults.FILL_OPACITY,
    @ColorInt color: Int = Defaults.FILL_COLOR,
    @ColorInt outlineColor: Int? = Defaults.FILL_OUTLINE_COLOR,
    pattern: Bitmap? = Defaults.FILL_PATTERN,
    /**
     * NDD
     */
    antialias: Boolean = Defaults.FILL_ANTIALIAS,
    /**
     * NDD
     */
    translate: Translate? = Defaults.FILL_TRANSLATE
) : KAnnotation<Polygon>() {

    var paths: List<List<LatLng>> = paths
        set(value) {
            field = value
            geometry = Polygon.fromLngLats(value.map { it.map { Point.fromLngLat(it.longitude, it.latitude) } })
            updateThis()
        }
    var opacity: Float = opacity
        set(value) {
            field = value
            updateThis()
        }
    @ColorInt var color: Int = color
        set(value) {
            field = value
            updateThis()
        }
    @ColorInt var outlineColor: Int? = outlineColor
        set(value) {
            field = value
            updateThis()
        }
    var pattern: Bitmap? = pattern
        set(value) {
            field = value
            updateThis()
        }
    var antialias: Boolean = antialias
        set(value) {
            field = value
            updateAll()
        }
    var translate: Translate? = translate
        set(value) {
            field = value
            updateAll()
        }

    override var clickListener: OnAnnotationClickListener<Fill>? = null
    override var longClickListener: OnAnnotationLongClickListener<Fill>? = null

    override var geometry: Polygon = Polygon.fromLngLats(paths.map { it.map { Point.fromLngLat(it.longitude, it.latitude) } })
    override val dataDrivenProperties: List<PairWithDefault>
        get() = listOf(
            PROPERTY_FILL_SORT_KEY to zLayer default Defaults.Z_LAYER,
            PROPERTY_FILL_OPACITY to opacity default Defaults.FILL_OPACITY,
            PROPERTY_FILL_COLOR to color.asColorString() default Defaults.FILL_COLOR.asColorString(),
            PROPERTY_FILL_OUTLINE_COLOR to outlineColor?.asColorString()
                    default Defaults.FILL_OUTLINE_COLOR?.asColorString(),
            PROPERTY_FILL_PATTERN to pattern default Defaults.FILL_PATTERN
        )

    override fun offsetGeometry(
        projection: Projection,
        moveDistancesObject: MoveDistancesObject,
        touchAreaShiftX: Float,
        touchAreaShiftY: Float
    ): Boolean =
        geometry.coordinates().map { innerList ->
            innerList.map {
                val pointF = projection.toScreenLocation(LatLng(it.latitude(), it.longitude())).apply {
                    x -= moveDistancesObject.distanceXSinceLast
                    y -= moveDistancesObject.distanceYSinceLast
                }

                val latLng = projection.fromScreenLocation(pointF)
                if (latLng.latitude > MAX_MERCATOR_LATITUDE || latLng.latitude < MIN_MERCATOR_LATITUDE) {
                    return false
                }
                Point.fromLngLat(latLng.longitude, latLng.latitude)
            }
        }.let { Polygon.fromLngLats(it) }?.let {
                geometry = it
                updateThis()
                true
            } ?: false

    init {
        if (opacity > 1f || opacity < 0f) {
            throw IllegalArgumentException("Opacity must be between 0 and 1 (inclusive)")
        }
    }

    companion object {
        internal const val PROPERTY_FILL_SORT_KEY = "fill-sort-key"
        internal const val PROPERTY_FILL_OPACITY = "fill-opacity"
        internal const val PROPERTY_FILL_COLOR = "fill-color"
        internal const val PROPERTY_FILL_OUTLINE_COLOR = "fill-outline-color"
        internal const val PROPERTY_FILL_PATTERN = "fill-pattern"
    }
}