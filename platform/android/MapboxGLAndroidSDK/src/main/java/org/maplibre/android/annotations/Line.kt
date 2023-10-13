package org.maplibre.android.annotations

import android.graphics.Bitmap
import androidx.annotation.ColorInt
import com.mapbox.android.gestures.MoveDistancesObject
import com.mapbox.geojson.Geometry
import com.mapbox.geojson.LineString
import com.mapbox.geojson.Point
import org.maplibre.android.annotations.data.Defaults
import org.maplibre.android.constants.GeometryConstants.MAX_MERCATOR_LATITUDE
import org.maplibre.android.constants.GeometryConstants.MIN_MERCATOR_LATITUDE
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.Projection

class Line @JvmOverloads constructor(
    path: List<LatLng>,
    join: Join = Defaults.LINE_JOIN,
    opacity: Float = Defaults.LINE_OPACITY,
    @ColorInt color: Int = Defaults.LINE_COLOR,
    width: Float = Defaults.LINE_WIDTH,
    gap: Float? = Defaults.LINE_GAP,
    offset: Float = Defaults.LINE_OFFSET,
    blur: Float? = Defaults.LINE_BLUR,
    pattern: Bitmap? = Defaults.LINE_PATTERN,
    // TODO: NDD properties cap, translate, dashArray
) : KAnnotation<LineString>() {

    var path: List<LatLng> = path
        set(value) {
            field = value
            geometry = LineString.fromLngLats(
                path.map { Point.fromLngLat(it.longitude, it.latitude) }
            )
            updateThis()
        }
    var join: Join = join
        set(value) {
            field = value
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
    var width: Float = width
        set(value) {
            field = value
            updateThis()
        }
    var gap: Float? = gap
        set(value) {
            field = value
            updateThis()
        }
    var offset: Float = offset
        set(value) {
            field = value
            updateThis()
        }
    var blur: Float? = blur
        set(value) {
            field = value
            updateThis()
        }
    var pattern: Bitmap? = pattern
        set(value) {
            field = value
            updateThis()
        }

    override var geometry: LineString = LineString.fromLngLats(
            path.map { Point.fromLngLat(it.longitude, it.latitude) }
        )

    override val dataDrivenProperties: List<PairWithDefault>
        get() = listOf(
            PROPERTY_IS_DRAGGABLE to draggable default Defaults.DRAGGABLE,
            PROPERTY_LINE_SORT_KEY to zLayer default Defaults.Z_LAYER,
            PROPERTY_LINE_JOIN to join.toString().lowercase() default Defaults.LINE_JOIN.toString().lowercase(),
            PROPERTY_LINE_OPACITY to opacity default Defaults.LINE_OPACITY,
            PROPERTY_LINE_COLOR to color.asColorString() default Defaults.LINE_COLOR,
            PROPERTY_LINE_WIDTH to width default Defaults.LINE_WIDTH,
            PROPERTY_LINE_GAP_WIDTH to gap default Defaults.LINE_GAP,
            PROPERTY_LINE_OFFSET to offset default Defaults.LINE_OFFSET,
            PROPERTY_LINE_BLUR to blur default Defaults.LINE_BLUR,
            PROPERTY_LINE_PATTERN to pattern default Defaults.LINE_PATTERN
        )

    override fun getOffsetGeometry(
        projection: Projection, moveDistancesObject: MoveDistancesObject, touchAreaShiftX: Float, touchAreaShiftY: Float
    ): Geometry? =
        geometry.coordinates().map {
            val pointF = projection.toScreenLocation(LatLng(it.latitude(), it.longitude())).apply {
                x -= moveDistancesObject.distanceXSinceLast
                y -= moveDistancesObject.distanceYSinceLast
            }

            val latLng = projection.fromScreenLocation(pointF)
            if (latLng.latitude > MAX_MERCATOR_LATITUDE || latLng.latitude < MIN_MERCATOR_LATITUDE) {
                return null
            }
            Point.fromLngLat(latLng.longitude, latLng.latitude)
        }.let { LineString.fromLngLats(it) }

    init {
        if (gap != null && gap <= 0) {
            throw IllegalArgumentException(
                "A gap of $gap has been provided for a Line object. This means that no line gap is to be used. " +
                        "Please use `null` to indicate that `gap` is not used."
            )
        }
        if (blur != null && blur <= 0) {
            throw IllegalArgumentException(
                "A blur of $blur has been provided for a Line object. This means that no blur is to be used. " +
                        "Please use `null` to indicate that `blur` is not used."
            )
        }
        if (opacity > 1f || opacity < 0f) {
            throw IllegalArgumentException("Opacity must be between 0 and 1 (inclusive)")
        }

    }

    enum class Join {
        BEVEL, ROUND, MITER
    }

    companion object {
        internal const val PROPERTY_LINE_SORT_KEY = "line-sort-key"
        internal const val PROPERTY_LINE_JOIN = "line-join"
        internal const val PROPERTY_LINE_OPACITY = "line-opacity"
        internal const val PROPERTY_LINE_COLOR = "line-color"
        internal const val PROPERTY_LINE_WIDTH = "line-width"
        internal const val PROPERTY_LINE_GAP_WIDTH = "line-gap-width"
        internal const val PROPERTY_LINE_OFFSET = "line-offset"
        internal const val PROPERTY_LINE_BLUR = "line-blur"
        internal const val PROPERTY_LINE_PATTERN = "line-pattern"
    }
}