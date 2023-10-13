package org.maplibre.android.annotations

import android.graphics.Bitmap
import androidx.annotation.ColorInt
import com.mapbox.android.gestures.MoveDistancesObject
import com.mapbox.geojson.Geometry
import com.mapbox.geojson.Point
import com.mapbox.geojson.Polygon
import org.maplibre.android.annotations.data.Defaults
import org.maplibre.android.constants.GeometryConstants.MAX_MERCATOR_LATITUDE
import org.maplibre.android.constants.GeometryConstants.MIN_MERCATOR_LATITUDE
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.Projection

class Fill @JvmOverloads constructor(
    paths: List<List<LatLng>>,
    opacity: Float = Defaults.FILL_OPACITY,
    @ColorInt color: Int = Defaults.FILL_COLOR,
    @ColorInt outlineColor: Int? = Defaults.FILL_OUTLINE_COLOR,
    pattern: Bitmap? = Defaults.FILL_PATTERN,
    // TODO: NDD properties antialias and translate
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

    override var geometry: Polygon = Polygon.fromLngLats(paths.map { it.map { Point.fromLngLat(it.longitude, it.latitude) } })
    override val dataDrivenProperties: List<PairWithDefault>
        get() = listOf(
            PROPERTY_IS_DRAGGABLE to draggable default Defaults.DRAGGABLE,
            PROPERTY_FILL_SORT_KEY to zLayer default Defaults.Z_LAYER,
            PROPERTY_FILL_OPACITY to opacity default Defaults.FILL_OPACITY,
            PROPERTY_FILL_COLOR to color.asColorString() default Defaults.FILL_COLOR.asColorString(),
            PROPERTY_FILL_OUTLINE_COLOR to outlineColor?.asColorString()
                    default Defaults.FILL_OUTLINE_COLOR?.asColorString(),
            PROPERTY_FILL_PATTERN to pattern default Defaults.FILL_PATTERN
        )

    override fun getOffsetGeometry(
        projection: Projection,
        moveDistancesObject: MoveDistancesObject,
        touchAreaShiftX: Float,
        touchAreaShiftY: Float
    ): Geometry? =
        geometry.coordinates().map { innerList ->
            innerList.map {
                val pointF = projection.toScreenLocation(LatLng(it.latitude(), it.longitude())).apply {
                    x -= moveDistancesObject.distanceXSinceLast
                    y -= moveDistancesObject.distanceYSinceLast
                }

                val latLng = projection.fromScreenLocation(pointF)
                if (latLng.latitude > MAX_MERCATOR_LATITUDE || latLng.latitude < MIN_MERCATOR_LATITUDE) {
                    return null
                }
                Point.fromLngLat(latLng.longitude, latLng.latitude)
            }
        }.let { Polygon.fromLngLats(it) }

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