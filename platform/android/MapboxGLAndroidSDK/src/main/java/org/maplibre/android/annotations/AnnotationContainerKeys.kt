package org.maplibre.android.annotations

import android.graphics.Paint
import org.maplibre.android.annotations.data.Alignment
import org.maplibre.android.annotations.data.Defaults
import org.maplibre.android.annotations.data.Icon
import org.maplibre.android.annotations.data.Translate

internal sealed class Key(val z: Int)
internal class SymbolKey(
    z: Int,
    val iconFitText: Icon.FitText,
    val iconKeepUpright: Boolean,
    val iconPitchAlignment: Alignment?,
    val textPitchAlignment: Alignment?,
    val textLineHeight: Float
) : Key(z)

internal class LineKey(
    z: Int,
    val cap: Paint.Cap,
    val translate: Translate?,
    val dashArray: Array<Float>?
) : Key(z)

internal class FillKey(
    z: Int,
    val antialias: Boolean,
    val translate: Translate?
) : Key(z)

internal class CircleKey(
    z: Int,
    val translate: Translate?,
    val pitchScale: Alignment,
    val pitchAlignment: Alignment
) : Key(z)

internal fun KAnnotation<*>.key() = when (this) {
    is Symbol -> SymbolKey(
        zLayer,
        icon?.fitText ?: Defaults.ICON_FIT_TEXT,
        icon?.keepUpright ?: Defaults.ICON_KEEP_UPRIGHT,
        icon?.pitchAlignment ?: Defaults.ICON_PITCH_ALIGNMENT,
        text?.pitchAlignment ?: Defaults.TEXT_PITCH_ALIGNMENT,
        text?.lineHeight ?: Defaults.TEXT_LINE_HEIGHT
    )
    is Line -> LineKey(
        zLayer, cap, translate, dashArray
    )
    is Fill -> FillKey(
        zLayer, antialias, translate
    )
    is Circle -> CircleKey(
        zLayer, translate, pitchScale, pitchAlignment
    )
}