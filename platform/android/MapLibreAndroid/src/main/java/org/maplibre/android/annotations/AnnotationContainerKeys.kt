package org.maplibre.android.annotations

import android.graphics.Paint
import org.maplibre.android.annotations.data.Alignment
import org.maplibre.android.annotations.data.Defaults
import org.maplibre.android.annotations.data.Icon
import org.maplibre.android.annotations.data.Translate
import org.maplibre.android.style.layers.Property

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

data class CollisionGroupKey(
    override val z: Int,
    val collisionGroup: CollisionGroup
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

internal fun CollisionGroup.key(): CollisionGroupKey = CollisionGroupKey(
    this.zLayer,
    this
)

internal fun SymbolKey.applyProperties(to: SymbolManager) {
    to.iconTextFit = iconFitText.let { fitText ->
        if (fitText.width && fitText.height) Property.ICON_TEXT_FIT_BOTH
        else if (fitText.width) Property.ICON_TEXT_FIT_WIDTH
        else if (fitText.height) Property.ICON_TEXT_FIT_HEIGHT
        else Property.ICON_TEXT_FIT_NONE
    }
    to.iconTextFitPadding = iconFitText.padding.let { padding ->
        arrayOf(padding.top, padding.right, padding.bottom, padding.left)
    }

    to.iconKeepUpright = iconKeepUpright
    to.iconPitchAlignment = when (iconPitchAlignment) {
        Alignment.MAP -> Property.ICON_PITCH_ALIGNMENT_MAP
        Alignment.VIEWPORT -> Property.ICON_PITCH_ALIGNMENT_VIEWPORT
        null -> Property.ICON_PITCH_ALIGNMENT_AUTO
    }

    to.textPitchAlignment = when (textPitchAlignment) {
        Alignment.MAP -> Property.TEXT_PITCH_ALIGNMENT_MAP
        Alignment.VIEWPORT -> Property.TEXT_PITCH_ALIGNMENT_VIEWPORT
        null -> Property.TEXT_PITCH_ALIGNMENT_AUTO
    }
    to.textLineHeight = textLineHeight

}