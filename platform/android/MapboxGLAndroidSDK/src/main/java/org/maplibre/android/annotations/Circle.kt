package org.maplibre.android.annotations

import android.graphics.PointF
import androidx.annotation.ColorInt
import com.mapbox.android.gestures.MoveDistancesObject
import com.mapbox.geojson.Point
import org.maplibre.android.annotations.data.Alignment
import org.maplibre.android.annotations.data.Defaults
import org.maplibre.android.annotations.data.Translate
import org.maplibre.android.constants.GeometryConstants
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.Projection

class Circle @JvmOverloads constructor(
    center: LatLng,
    radius: Float = Defaults.CIRCLE_RADIUS,
    @ColorInt color: Int = Defaults.CIRCLE_COLOR,
    blur: Float? = Defaults.CIRCLE_BLUR,
    opacity: Float = Defaults.CIRCLE_OPACITY,
    stroke: Stroke? = Defaults.CIRCLE_STROKE,
    translate: Translate? = Defaults.CIRCLE_TRANSLATE,
    pitchScale: Alignment = Defaults.CIRCLE_PITCH_SCALE,
    pitchAlignment: Alignment = Defaults.CIRCLE_PITCH_ALIGNMENT
) : KAnnotation<Point>() {
    var center: LatLng = center
        set(value) {
            field = value
            geometry = Point.fromLngLat(value.longitude, value.latitude)
            updateThis()
        }
    var radius: Float = radius
        set(value) {
            field = value
            updateThis()
        }
    @ColorInt var color: Int = color
        set(value) {
            field = value
            updateThis()
        }
    var blur: Float? = blur
        set(value) {
            field = value
            updateThis()
        }
    var opacity: Float = opacity
        set(value) {
            if (value > 1f || value < 0f) {
                throw IllegalArgumentException("Opacity must be between 0 and 1 (inclusive)")
            }
            field = value
            updateThis()
        }
    var stroke: Stroke? = stroke
        set(value) {
            field = value
            updateThis()
        }
    var translate: Translate? = translate
        set(value) {
            field = value
            updateAll()
        }
    var pitchScale: Alignment = pitchScale
        set(value) {
            field = value
            updateAll()
        }
    var pitchAlignment: Alignment = pitchAlignment
        set(value) {
            field = value
            updateAll()
        }

    override var clickListener: OnAnnotationClickListener<Circle>? = null
    override var longClickListener: OnAnnotationLongClickListener<Circle>? = null

    override var geometry: Point = Point.fromLngLat(center.longitude, center.latitude)

    override val dataDrivenProperties: List<PairWithDefault>
        get() = listOf(
            PROPERTY_CIRCLE_SORT_KEY to zLayer default Defaults.Z_LAYER,
            PROPERTY_CIRCLE_RADIUS to radius default Defaults.CIRCLE_RADIUS,
            PROPERTY_CIRCLE_COLOR to color.asColorString() default Defaults.CIRCLE_COLOR.asColorString(),
            PROPERTY_CIRCLE_BLUR to blur default Defaults.CIRCLE_BLUR,
            PROPERTY_CIRCLE_OPACITY to opacity default Defaults.CIRCLE_OPACITY,
        ) + (stroke?.flattenedValues ?: emptyList())

    init {
        if (blur != null && blur <= 0) {
            throw IllegalArgumentException(
                "A blur of $blur has been provided for a Circle object. This means that no blur is to be used. " +
                        "Please use `null` to indicate that `blur` is not used."
            )
        }
    }

    override fun offsetGeometry(
        projection: Projection,
        moveDistancesObject: MoveDistancesObject,
        touchAreaShiftX: Float,
        touchAreaShiftY: Float
    ): Boolean {
        val pointF = PointF(
            moveDistancesObject.currentX - touchAreaShiftX,
            moveDistancesObject.currentY - touchAreaShiftY
        )
        val latLng = projection.fromScreenLocation(pointF)
        return if (latLng.latitude > GeometryConstants.MAX_MERCATOR_LATITUDE || latLng.latitude < GeometryConstants.MIN_MERCATOR_LATITUDE) {
            null
        } else {
            Point.fromLngLat(latLng.longitude, latLng.latitude)
        }?.let {
            geometry = it
            updateThis()
            true
        } ?: false
    }

    data class Stroke(
        val width: Float,
        @ColorInt val color: Int = Defaults.CIRCLE_STROKE_COLOR,
        val opacity: Float = Defaults.CIRCLE_STROKE_OPACITY
    ) {
        init {
            if (opacity > 1f || opacity < 0f) {
                throw IllegalArgumentException("Opacity must be between 0 and 1 (inclusive)")
            }
        }

        val flattenedValues: List<PairWithDefault>
            get() = listOf(
                PROPERTY_CIRCLE_STROKE_WIDTH to width default Unit,
                PROPERTY_CIRCLE_STROKE_COLOR to color.asColorString()
                        default Defaults.CIRCLE_STROKE_COLOR.asColorString(),
                PROPERTY_CIRCLE_STROKE_OPACITY to opacity default Defaults.CIRCLE_STROKE_OPACITY
            )
    }

    companion object {
        internal const val PROPERTY_CIRCLE_SORT_KEY = "circle-sort-key"
        internal const val PROPERTY_CIRCLE_RADIUS = "circle-radius"
        internal const val PROPERTY_CIRCLE_COLOR = "circle-color"
        internal const val PROPERTY_CIRCLE_BLUR = "circle-blur"
        internal const val PROPERTY_CIRCLE_OPACITY = "circle-opacity"
        internal const val PROPERTY_CIRCLE_STROKE_WIDTH = "circle-stroke-width"
        internal const val PROPERTY_CIRCLE_STROKE_COLOR = "circle-stroke-color"
        internal const val PROPERTY_CIRCLE_STROKE_OPACITY = "circle-stroke-opacity"
    }
}