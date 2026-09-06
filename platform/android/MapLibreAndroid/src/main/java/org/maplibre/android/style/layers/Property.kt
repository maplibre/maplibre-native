// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.style.layers

import androidx.annotation.StringDef

/**
 * Paint/Layout properties for Layer
 */
object Property {

    // VISIBILITY: Whether this layer is displayed.

    /**
     * The layer is shown.
     */
    const val VISIBLE = "visible"

    /**
     * The layer is hidden.
     */
    const val NONE = "none"

    @StringDef(
        VISIBLE,
        NONE
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class VISIBILITY

    // LINE_CAP: The display of line endings.

    /**
     * A cap with a squared-off end which is drawn to the exact endpoint of the line.
     */
    const val LINE_CAP_BUTT = "butt"

    /**
     * A cap with a rounded end which is drawn beyond the endpoint of the line at a radius of one-half of the line's width and centered on the endpoint of the line.
     */
    const val LINE_CAP_ROUND = "round"

    /**
     * A cap with a squared-off end which is drawn beyond the endpoint of the line at a distance of one-half of the line's width.
     */
    const val LINE_CAP_SQUARE = "square"

    /**
     * The display of line endings.
     */
    @StringDef(
        LINE_CAP_BUTT,
        LINE_CAP_ROUND,
        LINE_CAP_SQUARE,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class LINE_CAP

    // LINE_JOIN: The display of lines when joining.

    /**
     * A join with a squared-off end which is drawn beyond the endpoint of the line at a distance of one-half of the line's width.
     */
    const val LINE_JOIN_BEVEL = "bevel"

    /**
     * A join with a rounded end which is drawn beyond the endpoint of the line at a radius of one-half of the line's width and centered on the endpoint of the line.
     */
    const val LINE_JOIN_ROUND = "round"

    /**
     * A join with a sharp, angled corner which is drawn with the outer sides beyond the endpoint of the path until they meet.
     */
    const val LINE_JOIN_MITER = "miter"

    /**
     * The display of lines when joining.
     */
    @StringDef(
        LINE_JOIN_BEVEL,
        LINE_JOIN_ROUND,
        LINE_JOIN_MITER,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class LINE_JOIN

    // SYMBOL_PLACEMENT: Label placement relative to its geometry.

    /**
     * The label is placed at the point where the geometry is located.
     */
    const val SYMBOL_PLACEMENT_POINT = "point"

    /**
     * The label is placed along the line of the geometry. Can only be used on LineString and Polygon geometries.
     */
    const val SYMBOL_PLACEMENT_LINE = "line"

    /**
     * The label is placed at the center of the line of the geometry. Can only be used on LineString and Polygon geometries. Note that a single feature in a vector tile may contain multiple line geometries.
     */
    const val SYMBOL_PLACEMENT_LINE_CENTER = "line-center"

    /**
     * Label placement relative to its geometry.
     */
    @StringDef(
        SYMBOL_PLACEMENT_POINT,
        SYMBOL_PLACEMENT_LINE,
        SYMBOL_PLACEMENT_LINE_CENTER,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class SYMBOL_PLACEMENT

    // SYMBOL_Z_ORDER: Determines whether overlapping symbols in the same layer are rendered in the order that they appear in the data source or by their y-position relative to the viewport. To control the order and prioritization of symbols otherwise, use `symbol-sort-key`.

    /**
     * Sorts symbols by symbol sort key if set. Otherwise, sorts symbols by their y-position relative to the viewport if [ICON_ALLOW_OVERLAP] or [TEXT_ALLOW_OVERLAP] is set to [TRUE] or [ICON_IGNORE_PLACEMENT] or [TEXT_IGNORE_PLACEMENT] is [FALSE].
     */
    const val SYMBOL_Z_ORDER_AUTO = "auto"

    /**
     * Sorts symbols by their y-position relative to the viewport if [ICON_ALLOW_OVERLAP] or [TEXT_ALLOW_OVERLAP] is set to [TRUE] or [ICON_IGNORE_PLACEMENT] or [TEXT_IGNORE_PLACEMENT] is [FALSE].
     */
    const val SYMBOL_Z_ORDER_VIEWPORT_Y = "viewport-y"

    /**
     * Sorts symbols by symbol sort key if set. Otherwise, no sorting is applied; symbols are rendered in the same order as the source data.
     */
    const val SYMBOL_Z_ORDER_SOURCE = "source"

    /**
     * Determines whether overlapping symbols in the same layer are rendered in the order that they appear in the data source or by their y-position relative to the viewport. To control the order and prioritization of symbols otherwise, use `symbol-sort-key`.
     */
    @StringDef(
        SYMBOL_Z_ORDER_AUTO,
        SYMBOL_Z_ORDER_VIEWPORT_Y,
        SYMBOL_Z_ORDER_SOURCE,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class SYMBOL_Z_ORDER

    // ICON_ROTATION_ALIGNMENT: In combination with `symbol-placement`, determines the rotation behavior of icons.

    /**
     * When [SYMBOL_PLACEMENT] is set to [Property.SYMBOL_PLACEMENT_POINT], aligns icons east-west. When [SYMBOL_PLACEMENT] is set to [Property.SYMBOL_PLACEMENT_LINE] or [Property.SYMBOL_PLACEMENT_LINE_CENTER], aligns icon x-axes with the line.
     */
    const val ICON_ROTATION_ALIGNMENT_MAP = "map"

    /**
     * Produces icons whose x-axes are aligned with the x-axis of the viewport, regardless of the value of [SYMBOL_PLACEMENT].
     */
    const val ICON_ROTATION_ALIGNMENT_VIEWPORT = "viewport"

    /**
     * When [SYMBOL_PLACEMENT] is set to [Property.SYMBOL_PLACEMENT_POINT], this is equivalent to [Property.ICON_ROTATION_ALIGNMENT_VIEWPORT]. When [SYMBOL_PLACEMENT] is set to [Property.SYMBOL_PLACEMENT_LINE] or [Property.SYMBOL_PLACEMENT_LINE_CENTER], this is equivalent to [Property.ICON_ROTATION_ALIGNMENT_MAP].
     */
    const val ICON_ROTATION_ALIGNMENT_AUTO = "auto"

    /**
     * In combination with `symbol-placement`, determines the rotation behavior of icons.
     */
    @StringDef(
        ICON_ROTATION_ALIGNMENT_MAP,
        ICON_ROTATION_ALIGNMENT_VIEWPORT,
        ICON_ROTATION_ALIGNMENT_AUTO,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class ICON_ROTATION_ALIGNMENT

    // ICON_TEXT_FIT: Scales the icon to fit around the associated text.

    /**
     * The icon is displayed at its intrinsic aspect ratio.
     */
    const val ICON_TEXT_FIT_NONE = "none"

    /**
     * The icon is scaled in the x-dimension to fit the width of the text.
     */
    const val ICON_TEXT_FIT_WIDTH = "width"

    /**
     * The icon is scaled in the y-dimension to fit the height of the text.
     */
    const val ICON_TEXT_FIT_HEIGHT = "height"

    /**
     * The icon is scaled in both x- and y-dimensions.
     */
    const val ICON_TEXT_FIT_BOTH = "both"

    /**
     * Scales the icon to fit around the associated text.
     */
    @StringDef(
        ICON_TEXT_FIT_NONE,
        ICON_TEXT_FIT_WIDTH,
        ICON_TEXT_FIT_HEIGHT,
        ICON_TEXT_FIT_BOTH,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class ICON_TEXT_FIT

    // ICON_ANCHOR: Part of the icon placed closest to the anchor.

    /**
     * The center of the icon is placed closest to the anchor.
     */
    const val ICON_ANCHOR_CENTER = "center"

    /**
     * The left side of the icon is placed closest to the anchor.
     */
    const val ICON_ANCHOR_LEFT = "left"

    /**
     * The right side of the icon is placed closest to the anchor.
     */
    const val ICON_ANCHOR_RIGHT = "right"

    /**
     * The top of the icon is placed closest to the anchor.
     */
    const val ICON_ANCHOR_TOP = "top"

    /**
     * The bottom of the icon is placed closest to the anchor.
     */
    const val ICON_ANCHOR_BOTTOM = "bottom"

    /**
     * The top left corner of the icon is placed closest to the anchor.
     */
    const val ICON_ANCHOR_TOP_LEFT = "top-left"

    /**
     * The top right corner of the icon is placed closest to the anchor.
     */
    const val ICON_ANCHOR_TOP_RIGHT = "top-right"

    /**
     * The bottom left corner of the icon is placed closest to the anchor.
     */
    const val ICON_ANCHOR_BOTTOM_LEFT = "bottom-left"

    /**
     * The bottom right corner of the icon is placed closest to the anchor.
     */
    const val ICON_ANCHOR_BOTTOM_RIGHT = "bottom-right"

    /**
     * Part of the icon placed closest to the anchor.
     */
    @StringDef(
        ICON_ANCHOR_CENTER,
        ICON_ANCHOR_LEFT,
        ICON_ANCHOR_RIGHT,
        ICON_ANCHOR_TOP,
        ICON_ANCHOR_BOTTOM,
        ICON_ANCHOR_TOP_LEFT,
        ICON_ANCHOR_TOP_RIGHT,
        ICON_ANCHOR_BOTTOM_LEFT,
        ICON_ANCHOR_BOTTOM_RIGHT,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class ICON_ANCHOR

    // ICON_PITCH_ALIGNMENT: Orientation of icon when map is pitched.

    /**
     * The icon is aligned to the plane of the map.
     */
    const val ICON_PITCH_ALIGNMENT_MAP = "map"

    /**
     * The icon is aligned to the plane of the viewport.
     */
    const val ICON_PITCH_ALIGNMENT_VIEWPORT = "viewport"

    /**
     * Automatically matches the value of [ICON_ROTATION_ALIGNMENT].
     */
    const val ICON_PITCH_ALIGNMENT_AUTO = "auto"

    /**
     * Orientation of icon when map is pitched.
     */
    @StringDef(
        ICON_PITCH_ALIGNMENT_MAP,
        ICON_PITCH_ALIGNMENT_VIEWPORT,
        ICON_PITCH_ALIGNMENT_AUTO,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class ICON_PITCH_ALIGNMENT

    // TEXT_PITCH_ALIGNMENT: Orientation of text when map is pitched.

    /**
     * The text is aligned to the plane of the map.
     */
    const val TEXT_PITCH_ALIGNMENT_MAP = "map"

    /**
     * The text is aligned to the plane of the viewport.
     */
    const val TEXT_PITCH_ALIGNMENT_VIEWPORT = "viewport"

    /**
     * Automatically matches the value of [TEXT_ROTATION_ALIGNMENT].
     */
    const val TEXT_PITCH_ALIGNMENT_AUTO = "auto"

    /**
     * Orientation of text when map is pitched.
     */
    @StringDef(
        TEXT_PITCH_ALIGNMENT_MAP,
        TEXT_PITCH_ALIGNMENT_VIEWPORT,
        TEXT_PITCH_ALIGNMENT_AUTO,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class TEXT_PITCH_ALIGNMENT

    // TEXT_ROTATION_ALIGNMENT: In combination with `symbol-placement`, determines the rotation behavior of the individual glyphs forming the text.

    /**
     * When [SYMBOL_PLACEMENT] is set to [Property.SYMBOL_PLACEMENT_POINT], aligns text east-west. When [SYMBOL_PLACEMENT] is set to [Property.SYMBOL_PLACEMENT_LINE] or [Property.SYMBOL_PLACEMENT_LINE_CENTER], aligns text x-axes with the line.
     */
    const val TEXT_ROTATION_ALIGNMENT_MAP = "map"

    /**
     * Produces glyphs whose x-axes are aligned with the x-axis of the viewport, regardless of the value of [SYMBOL_PLACEMENT].
     */
    const val TEXT_ROTATION_ALIGNMENT_VIEWPORT = "viewport"

    /**
     * When [SYMBOL_PLACEMENT] is set to [Property.SYMBOL_PLACEMENT_POINT], this is equivalent to [Property.TEXT_ROTATION_ALIGNMENT_VIEWPORT]. When [SYMBOL_PLACEMENT] is set to [Property.SYMBOL_PLACEMENT_LINE] or [Property.SYMBOL_PLACEMENT_LINE_CENTER], this is equivalent to [Property.TEXT_ROTATION_ALIGNMENT_MAP].
     */
    const val TEXT_ROTATION_ALIGNMENT_AUTO = "auto"

    /**
     * In combination with `symbol-placement`, determines the rotation behavior of the individual glyphs forming the text.
     */
    @StringDef(
        TEXT_ROTATION_ALIGNMENT_MAP,
        TEXT_ROTATION_ALIGNMENT_VIEWPORT,
        TEXT_ROTATION_ALIGNMENT_AUTO,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class TEXT_ROTATION_ALIGNMENT

    // TEXT_JUSTIFY: Text justification options.

    /**
     * The text is aligned towards the anchor position.
     */
    const val TEXT_JUSTIFY_AUTO = "auto"

    /**
     * The text is aligned to the left.
     */
    const val TEXT_JUSTIFY_LEFT = "left"

    /**
     * The text is centered.
     */
    const val TEXT_JUSTIFY_CENTER = "center"

    /**
     * The text is aligned to the right.
     */
    const val TEXT_JUSTIFY_RIGHT = "right"

    /**
     * Text justification options.
     */
    @StringDef(
        TEXT_JUSTIFY_AUTO,
        TEXT_JUSTIFY_LEFT,
        TEXT_JUSTIFY_CENTER,
        TEXT_JUSTIFY_RIGHT,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class TEXT_JUSTIFY

    // TEXT_ANCHOR: Part of the text placed closest to the anchor.

    /**
     * The center of the text is placed closest to the anchor.
     */
    const val TEXT_ANCHOR_CENTER = "center"

    /**
     * The left side of the text is placed closest to the anchor.
     */
    const val TEXT_ANCHOR_LEFT = "left"

    /**
     * The right side of the text is placed closest to the anchor.
     */
    const val TEXT_ANCHOR_RIGHT = "right"

    /**
     * The top of the text is placed closest to the anchor.
     */
    const val TEXT_ANCHOR_TOP = "top"

    /**
     * The bottom of the text is placed closest to the anchor.
     */
    const val TEXT_ANCHOR_BOTTOM = "bottom"

    /**
     * The top left corner of the text is placed closest to the anchor.
     */
    const val TEXT_ANCHOR_TOP_LEFT = "top-left"

    /**
     * The top right corner of the text is placed closest to the anchor.
     */
    const val TEXT_ANCHOR_TOP_RIGHT = "top-right"

    /**
     * The bottom left corner of the text is placed closest to the anchor.
     */
    const val TEXT_ANCHOR_BOTTOM_LEFT = "bottom-left"

    /**
     * The bottom right corner of the text is placed closest to the anchor.
     */
    const val TEXT_ANCHOR_BOTTOM_RIGHT = "bottom-right"

    /**
     * Part of the text placed closest to the anchor.
     */
    @StringDef(
        TEXT_ANCHOR_CENTER,
        TEXT_ANCHOR_LEFT,
        TEXT_ANCHOR_RIGHT,
        TEXT_ANCHOR_TOP,
        TEXT_ANCHOR_BOTTOM,
        TEXT_ANCHOR_TOP_LEFT,
        TEXT_ANCHOR_TOP_RIGHT,
        TEXT_ANCHOR_BOTTOM_LEFT,
        TEXT_ANCHOR_BOTTOM_RIGHT,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class TEXT_ANCHOR

    // TEXT_TRANSFORM: Specifies how to capitalize text, similar to the CSS `text-transform` property.

    /**
     * The text is not altered.
     */
    const val TEXT_TRANSFORM_NONE = "none"

    /**
     * Forces all letters to be displayed in uppercase.
     */
    const val TEXT_TRANSFORM_UPPERCASE = "uppercase"

    /**
     * Forces all letters to be displayed in lowercase.
     */
    const val TEXT_TRANSFORM_LOWERCASE = "lowercase"

    /**
     * Specifies how to capitalize text, similar to the CSS `text-transform` property.
     */
    @StringDef(
        TEXT_TRANSFORM_NONE,
        TEXT_TRANSFORM_UPPERCASE,
        TEXT_TRANSFORM_LOWERCASE,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class TEXT_TRANSFORM

    // FILL_TRANSLATE_ANCHOR: Controls the frame of reference for `fill-translate`.

    /**
     * The fill is translated relative to the map.
     */
    const val FILL_TRANSLATE_ANCHOR_MAP = "map"

    /**
     * The fill is translated relative to the viewport.
     */
    const val FILL_TRANSLATE_ANCHOR_VIEWPORT = "viewport"

    /**
     * Controls the frame of reference for `fill-translate`.
     */
    @StringDef(
        FILL_TRANSLATE_ANCHOR_MAP,
        FILL_TRANSLATE_ANCHOR_VIEWPORT,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class FILL_TRANSLATE_ANCHOR

    // LINE_TRANSLATE_ANCHOR: Controls the frame of reference for `line-translate`.

    /**
     * The line is translated relative to the map.
     */
    const val LINE_TRANSLATE_ANCHOR_MAP = "map"

    /**
     * The line is translated relative to the viewport.
     */
    const val LINE_TRANSLATE_ANCHOR_VIEWPORT = "viewport"

    /**
     * Controls the frame of reference for `line-translate`.
     */
    @StringDef(
        LINE_TRANSLATE_ANCHOR_MAP,
        LINE_TRANSLATE_ANCHOR_VIEWPORT,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class LINE_TRANSLATE_ANCHOR

    // ICON_TRANSLATE_ANCHOR: Controls the frame of reference for `icon-translate`.

    /**
     * Icons are translated relative to the map.
     */
    const val ICON_TRANSLATE_ANCHOR_MAP = "map"

    /**
     * Icons are translated relative to the viewport.
     */
    const val ICON_TRANSLATE_ANCHOR_VIEWPORT = "viewport"

    /**
     * Controls the frame of reference for `icon-translate`.
     */
    @StringDef(
        ICON_TRANSLATE_ANCHOR_MAP,
        ICON_TRANSLATE_ANCHOR_VIEWPORT,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class ICON_TRANSLATE_ANCHOR

    // TEXT_TRANSLATE_ANCHOR: Controls the frame of reference for `text-translate`.

    /**
     * The text is translated relative to the map.
     */
    const val TEXT_TRANSLATE_ANCHOR_MAP = "map"

    /**
     * The text is translated relative to the viewport.
     */
    const val TEXT_TRANSLATE_ANCHOR_VIEWPORT = "viewport"

    /**
     * Controls the frame of reference for `text-translate`.
     */
    @StringDef(
        TEXT_TRANSLATE_ANCHOR_MAP,
        TEXT_TRANSLATE_ANCHOR_VIEWPORT,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class TEXT_TRANSLATE_ANCHOR

    // CIRCLE_TRANSLATE_ANCHOR: Controls the frame of reference for `circle-translate`.

    /**
     * The circle is translated relative to the map.
     */
    const val CIRCLE_TRANSLATE_ANCHOR_MAP = "map"

    /**
     * The circle is translated relative to the viewport.
     */
    const val CIRCLE_TRANSLATE_ANCHOR_VIEWPORT = "viewport"

    /**
     * Controls the frame of reference for `circle-translate`.
     */
    @StringDef(
        CIRCLE_TRANSLATE_ANCHOR_MAP,
        CIRCLE_TRANSLATE_ANCHOR_VIEWPORT,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class CIRCLE_TRANSLATE_ANCHOR

    // CIRCLE_PITCH_SCALE: Controls the scaling behavior of the circle when the map is pitched.

    /**
     * Circles are scaled according to their apparent distance to the camera.
     */
    const val CIRCLE_PITCH_SCALE_MAP = "map"

    /**
     * Circles are not scaled.
     */
    const val CIRCLE_PITCH_SCALE_VIEWPORT = "viewport"

    /**
     * Controls the scaling behavior of the circle when the map is pitched.
     */
    @StringDef(
        CIRCLE_PITCH_SCALE_MAP,
        CIRCLE_PITCH_SCALE_VIEWPORT,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class CIRCLE_PITCH_SCALE

    // CIRCLE_PITCH_ALIGNMENT: Orientation of circle when map is pitched.

    /**
     * The circle is aligned to the plane of the map.
     */
    const val CIRCLE_PITCH_ALIGNMENT_MAP = "map"

    /**
     * The circle is aligned to the plane of the viewport.
     */
    const val CIRCLE_PITCH_ALIGNMENT_VIEWPORT = "viewport"

    /**
     * Orientation of circle when map is pitched.
     */
    @StringDef(
        CIRCLE_PITCH_ALIGNMENT_MAP,
        CIRCLE_PITCH_ALIGNMENT_VIEWPORT,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class CIRCLE_PITCH_ALIGNMENT

    // FILL_EXTRUSION_TRANSLATE_ANCHOR: Controls the frame of reference for `fill-extrusion-translate`.

    /**
     * The fill extrusion is translated relative to the map.
     */
    const val FILL_EXTRUSION_TRANSLATE_ANCHOR_MAP = "map"

    /**
     * The fill extrusion is translated relative to the viewport.
     */
    const val FILL_EXTRUSION_TRANSLATE_ANCHOR_VIEWPORT = "viewport"

    /**
     * Controls the frame of reference for `fill-extrusion-translate`.
     */
    @StringDef(
        FILL_EXTRUSION_TRANSLATE_ANCHOR_MAP,
        FILL_EXTRUSION_TRANSLATE_ANCHOR_VIEWPORT,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class FILL_EXTRUSION_TRANSLATE_ANCHOR

    // RASTER_RESAMPLING: The resampling/interpolation method to use for overscaling, also known as texture magnification filter

    /**
     * (Bi)linear filtering interpolates pixel values using the weighted average of the four closest original source pixels creating a smooth but blurry look when overscaled
     */
    const val RASTER_RESAMPLING_LINEAR = "linear"

    /**
     * Nearest neighbor filtering interpolates pixel values using the nearest original source pixel creating a sharp but pixelated look when overscaled
     */
    const val RASTER_RESAMPLING_NEAREST = "nearest"

    /**
     * The resampling/interpolation method to use for overscaling, also known as texture magnification filter
     */
    @StringDef(
        RASTER_RESAMPLING_LINEAR,
        RASTER_RESAMPLING_NEAREST,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class RASTER_RESAMPLING

    // HILLSHADE_ILLUMINATION_ANCHOR: Direction of light source when map is rotated.

    /**
     * The hillshade illumination is relative to the north direction.
     */
    const val HILLSHADE_ILLUMINATION_ANCHOR_MAP = "map"

    /**
     * The hillshade illumination is relative to the top of the viewport.
     */
    const val HILLSHADE_ILLUMINATION_ANCHOR_VIEWPORT = "viewport"

    /**
     * Direction of light source when map is rotated.
     */
    @StringDef(
        HILLSHADE_ILLUMINATION_ANCHOR_MAP,
        HILLSHADE_ILLUMINATION_ANCHOR_VIEWPORT,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class HILLSHADE_ILLUMINATION_ANCHOR

    // HILLSHADE_METHOD: The hillshade algorithm to use, one of `standard`, `basic`, `combined`, `igor`, or `multidirectional`. ![image](assets/hillshade_methods.png)

    /**
     * The legacy hillshade method.
     */
    const val HILLSHADE_METHOD_STANDARD = "standard"

    /**
     * Basic hillshade. Uses a simple physics model where the reflected light intensity is proportional to the cosine of the angle between the incident light and the surface normal. Similar to GDAL's [GDALDEM] default algorithm.
     */
    const val HILLSHADE_METHOD_BASIC = "basic"

    /**
     * Hillshade algorithm whose intensity scales with slope. Similar to GDAL's [GDALDEM] with -combined option.
     */
    const val HILLSHADE_METHOD_COMBINED = "combined"

    /**
     * Hillshade algorithm which tries to minimize effects on other map features beneath. Similar to GDAL's [GDALDEM] with -igor option.
     */
    const val HILLSHADE_METHOD_IGOR = "igor"

    /**
     * Hillshade with multiple illumination directions. Uses the basic hillshade model with multiple independent light sources.
     */
    const val HILLSHADE_METHOD_MULTIDIRECTIONAL = "multidirectional"

    /**
     * The hillshade algorithm to use, one of `standard`, `basic`, `combined`, `igor`, or `multidirectional`. ![image](assets/hillshade_methods.png)
     */
    @StringDef(
        HILLSHADE_METHOD_STANDARD,
        HILLSHADE_METHOD_BASIC,
        HILLSHADE_METHOD_COMBINED,
        HILLSHADE_METHOD_IGOR,
        HILLSHADE_METHOD_MULTIDIRECTIONAL,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class HILLSHADE_METHOD

    // ANCHOR: Whether extruded geometries are lit relative to the map or viewport.

    /**
     * The position of the light source is aligned to the rotation of the map.
     */
    const val ANCHOR_MAP = "map"

    /**
     * The position of the light source is aligned to the rotation of the viewport.
     */
    const val ANCHOR_VIEWPORT = "viewport"

    /**
     * Whether extruded geometries are lit relative to the map or viewport.
     */
    @StringDef(
        ANCHOR_MAP,
        ANCHOR_VIEWPORT,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class ANCHOR

    // TEXT_WRITING_MODE: The property allows control over a symbol's orientation. Note that the property values act as a hint, so that a symbol whose language doesn’t support the provided orientation will be laid out in its natural orientation. Example: English point symbol will be rendered horizontally even if array value contains single 'vertical' enum value. The order of elements in an array define priority order for the placement of an orientation variant.

    /**
     * If a text's language supports horizontal writing mode, symbols with point placement would be laid out horizontally.
     */
    const val TEXT_WRITING_MODE_HORIZONTAL = "horizontal"

    /**
     * If a text's language supports vertical writing mode, symbols with point placement would be laid out vertically.
     */
    const val TEXT_WRITING_MODE_VERTICAL = "vertical"

    /**
     * The property allows control over a symbol's orientation. Note that the property values act as a hint, so that a symbol whose language doesn’t support the provided orientation will be laid out in its natural orientation. Example: English point symbol will be rendered horizontally even if array value contains single 'vertical' enum value. The order of elements in an array define priority order for the placement of an orientation variant.
     */
    @StringDef(
        TEXT_WRITING_MODE_HORIZONTAL,
        TEXT_WRITING_MODE_VERTICAL,
    )
    @Retention(AnnotationRetention.SOURCE)
    annotation class TEXT_WRITING_MODE

}
