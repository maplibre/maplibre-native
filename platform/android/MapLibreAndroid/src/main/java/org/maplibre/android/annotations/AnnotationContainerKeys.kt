package org.maplibre.android.annotations

import android.graphics.Paint
import org.maplibre.android.annotations.data.Alignment
import org.maplibre.android.annotations.data.Defaults
import org.maplibre.android.annotations.data.Icon
import org.maplibre.android.annotations.data.Translate

internal sealed interface Key {
    val z: Int
}
internal data class SymbolKey(
    override val z: Int,
    val iconFitText: Icon.FitText,
    val iconKeepUpright: Boolean,
    val iconPitchAlignment: Alignment?,
    val textPitchAlignment: Alignment?,
    val textLineHeight: Float
) : Key

internal data class LineKey(
    override val z: Int,
    val cap: Paint.Cap,
    val translate: Translate?,
    val dashArray: List<Float>?
) : Key

internal data class FillKey(
    override val z: Int,
    val antialias: Boolean,
    val translate: Translate?
) : Key

internal data class CircleKey(
    override val z: Int,
    val translate: Translate?,
    val pitchScale: Alignment,
    val pitchAlignment: Alignment
) : Key

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