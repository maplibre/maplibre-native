package org.maplibre.android.annotations

import android.graphics.Bitmap
import android.graphics.Paint.Cap
import androidx.annotation.ColorInt
import com.mapbox.android.gestures.MoveDistancesObject
import com.mapbox.geojson.Geometry
import com.mapbox.geojson.LineString
import com.mapbox.geojson.Point
import org.maplibre.android.annotations.data.Defaults
import org.maplibre.android.annotations.data.Translate
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
    /**
     * NDD
     */
    cap: Cap = Defaults.LINE_CAP,
    /**
     * NDD
     */
    translate: Translate? = Defaults.LINE_TRANSLATE,
    /**
     * NDD
     */
    dashArray: Array<Float>? = Defaults.LINE_DASH_ARRAY
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
            if (value != null && value <= 0) {
                throw IllegalArgumentException(
                    "You tried setting a gap of $gap for a Line object. This means that no line gap is to be used. " +
                            "Please use `null` to indicate that `gap` is not used."
                )
            }
            field = value
            updateThis()
        }
    var offset: Float = offset
        set(value) {
            if (value > 1f || value < 0f) {
                throw IllegalArgumentException("Opacity must be between 0 and 1 (inclusive)")
            }
            field = value
            updateThis()
        }
    var blur: Float? = blur
        set(value) {
            if (value != null && value <= 0) {
                throw IllegalArgumentException(
                    "You tried setting a blur of $blur for a Line object. This means that no blur is to be used. " +
                            "Please use `null` to indicate that `blur` is not used."
                )
            }
            field = value
            updateThis()
        }
    var pattern: Bitmap? = pattern
        set(value) {
            field = value
            updateThis()
        }
    var cap: Cap? = cap
        set(value) {
            field = value
            updateAll()
        }
    var translate: Translate? = translate
        set(value) {
            field = value
            updateAll()
        }
    var dashArray: Array<Float>? = dashArray
        set(value) {
            if (value != null && value.size % 2 != 0) {
                throw IllegalArgumentException(
                    "You attempted setting a dash array of the uneven size ${value.size}. Dash arrays " +
                            "need an even amount of entries."
                )
            }
            field = value
            updateAll()
        }

    override var clickListener: OnAnnotationClickListener<Line>? = null
    override var longClickListener: OnAnnotationLongClickListener<Line>? = null

    override var geometry: LineString = LineString.fromLngLats(
            path.map { Point.fromLngLat(it.longitude, it.latitude) }
        )

    override val dataDrivenProperties: List<PairWithDefault>
        get() = listOf(
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

    override fun offsetGeometry(
        projection: Projection, moveDistancesObject: MoveDistancesObject, touchAreaShiftX: Float, touchAreaShiftY: Float
    ): Boolean =
        geometry.coordinates().map {
            val pointF = projection.toScreenLocation(LatLng(it.latitude(), it.longitude())).apply {
                x -= moveDistancesObject.distanceXSinceLast
                y -= moveDistancesObject.distanceYSinceLast
            }

            val latLng = projection.fromScreenLocation(pointF)
            if (latLng.latitude > MAX_MERCATOR_LATITUDE || latLng.latitude < MIN_MERCATOR_LATITUDE) {
                return false
            }
            Point.fromLngLat(latLng.longitude, latLng.latitude)
        }.let { LineString.fromLngLats(it) }?.let {
            geometry = it
            updateThis()
            true
        } ?: false

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
        if (dashArray != null && dashArray.size % 2 != 0) {
            throw IllegalArgumentException(
                "A dash array of the uneven size ${dashArray.size} has been provided. Dash arrays need an even " +
                        "amount of entries."
            )
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