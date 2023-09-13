package org.maplibre.android.annotations

import android.graphics.PointF
import com.mapbox.android.gestures.MoveDistancesObject
import com.mapbox.geojson.Geometry
import com.mapbox.geojson.Point
import org.maplibre.android.annotations.data.Defaults
import org.maplibre.android.annotations.data.Icon
import org.maplibre.android.annotations.data.Text
import org.maplibre.android.constants.GeometryConstants
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.Projection

class KSymbol(
    position: LatLng,
    icon: Icon? = Defaults.SYMBOL_ICON,
    text: Text? = Defaults.SYMBOL_TEXT
) : KAnnotation<Point>() {
    var position: LatLng = position
        set(value) {
            field = value
            updateThis()
        }
    var icon: Icon? = icon
        set(value) {
            field = value
            updateThis()
        }
    var text: Text? = text
        set(value) {
            field = value
            updateThis()
        }

    override var geometry: Point = Point.fromLngLat(position.longitude, position.latitude)

    override val dataDrivenProperties: List<PairWithDefault>
        get() = listOfNotNull(icon?.flattenedValues, text?.flattenedValues).flatten() +
            listOf(
                PROPERTY_SYMBOL_SORT_KEY to zLayer default Defaults.Z_LAYER,
                PROPERTY_IS_DRAGGABLE to draggable default Defaults.DRAGGABLE
            )

    override fun getOffsetGeometry(
        projection: Projection,
        moveDistancesObject: MoveDistancesObject,
        touchAreaShiftX: Float,
        touchAreaShiftY: Float
    ): Geometry? {
        val pointF = PointF(
            moveDistancesObject.currentX - touchAreaShiftX,
            moveDistancesObject.currentY - touchAreaShiftY
        )
        val latLng = projection.fromScreenLocation(pointF)
        return if (latLng.latitude > GeometryConstants.MAX_MERCATOR_LATITUDE || latLng.latitude < GeometryConstants.MIN_MERCATOR_LATITUDE) {
            null
        } else {
            Point.fromLngLat(latLng.longitude, latLng.latitude)
        }
    }

    companion object {
        internal const val PROPERTY_SYMBOL_SORT_KEY = "symbol-sort-key"
        internal const val PROPERTY_ICON_SIZE = "icon-size"
        internal const val PROPERTY_ICON_IMAGE = "icon-image"
        internal const val PROPERTY_ICON_ROTATE = "icon-rotate"
        internal const val PROPERTY_ICON_OFFSET = "icon-offset"
        internal const val PROPERTY_ICON_ANCHOR = "icon-anchor"
        internal const val PROPERTY_TEXT_FIELD = "text-field"
        internal const val PROPERTY_TEXT_FONT = "text-font"
        internal const val PROPERTY_TEXT_SIZE = "text-size"
        internal const val PROPERTY_TEXT_MAX_WIDTH = "text-max-width"
        internal const val PROPERTY_TEXT_LETTER_SPACING = "text-letter-spacing"
        internal const val PROPERTY_TEXT_JUSTIFY = "text-justify"
        internal const val PROPERTY_TEXT_RADIAL_OFFSET = "text-radial-offset"
        internal const val PROPERTY_TEXT_ANCHOR = "text-anchor"
        internal const val PROPERTY_TEXT_ROTATE = "text-rotate"
        internal const val PROPERTY_TEXT_TRANSFORM = "text-transform"
        internal const val PROPERTY_TEXT_OFFSET = "text-offset"
        internal const val PROPERTY_ICON_OPACITY = "icon-opacity"
        internal const val PROPERTY_ICON_COLOR = "icon-color"
        internal const val PROPERTY_ICON_HALO_COLOR = "icon-halo-color"
        internal const val PROPERTY_ICON_HALO_WIDTH = "icon-halo-width"
        internal const val PROPERTY_ICON_HALO_BLUR = "icon-halo-blur"
        internal const val PROPERTY_TEXT_OPACITY = "text-opacity"
        internal const val PROPERTY_TEXT_COLOR = "text-color"
        internal const val PROPERTY_TEXT_HALO_COLOR = "text-halo-color"
        internal const val PROPERTY_TEXT_HALO_WIDTH = "text-halo-width"
        internal const val PROPERTY_TEXT_HALO_BLUR = "text-halo-blur"
        internal const val PROPERTY_IS_DRAGGABLE = "is-draggable"
    }
}
