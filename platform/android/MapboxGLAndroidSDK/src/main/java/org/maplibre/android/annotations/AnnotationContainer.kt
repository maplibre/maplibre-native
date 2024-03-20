package org.maplibre.android.annotations

import android.graphics.Bitmap
import android.graphics.Paint.Cap
import androidx.annotation.UiThread
import org.maplibre.android.annotations.data.Alignment
import org.maplibre.android.annotations.data.Icon
import org.maplibre.android.annotations.data.Translate
import org.maplibre.android.annotations.data.toArray
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.style.layers.Property
import kotlin.reflect.KClass

/**
 * Has logic for spawning annotation managers for its collection of annotations.
 */
class KAnnotationContainer(
    private val mapLibreMap: MapLibreMap,
    private val mapView: MapView,
    private var style: Style?
) {

    private val annotationList: MutableList<KAnnotation<*>> = mutableListOf()
    private val managers: MutableMap<Key, AnnotationManager<*, *>> = mutableMapOf()
    private val bitmapUsers: MutableMap<Bitmap, MutableSet<KAnnotation<*>>> = mutableMapOf()

    @JvmName("setStyle")
    internal fun setStyle(style: Style) {
        this.style = style
        updateAll()
    }

    @UiThread
    fun add(annotation: KAnnotation<*>) {
        annotationList.add(annotation)
        annotation.icon()?.add(forAnnotation = annotation)
        addToManager(annotation)
    }

    fun KAnnotation<*>.icon(): Bitmap? = when (this) {
        is Symbol -> icon?.image
        is Line -> pattern
        is Fill -> pattern
        else -> null
    }

    fun Bitmap.add(forAnnotation: KAnnotation<*>) {
        style?.addImage(toString(), this)
        bitmapUsers.getOrPut(this) { mutableSetOf() }.add(forAnnotation)
    }

    @UiThread
    fun updateAll() {
        managers.values.forEach {
            it.onDestroy()
        }
        managers.clear()
        annotationList.forEach {
            addToManager(it)
        }
    }

    @UiThread
    fun update(annotation: KAnnotation<*>) {
        managers[annotation.key()]?.updateSource()
        if (annotation is Symbol) annotation.icon?.let { style?.addImage(it.image.toString(), it.image) }
        if (annotation is Line) annotation.pattern?.let { style?.addImage(it.toString(), it) }
        if (annotation is Fill) annotation.pattern?.let { style?.addImage(it.toString(), it) }
    }

    private fun addToManager(annotation: KAnnotation<*>) =
        managers.getOrCreate(annotation.key())?.let { manager ->
            when (annotation) {
                is Symbol -> (manager as SymbolManager).add(annotation)
                is Circle -> (manager as CircleManager).add(annotation)
                is Line -> (manager as LineManager).add(annotation)
                is Fill -> (manager as FillManager).add(annotation)
            }
        }

    @UiThread
    fun remove(annotation: KAnnotation<*>) {

        if (annotationList.remove(annotation)) {

            // Delete annotation from manager
            managers[annotation.key()]?.let { manager ->
                when (annotation) {
                    is Symbol -> (manager as SymbolManager).delete(annotation)
                    is Circle -> (manager as CircleManager).delete(annotation)
                    is Line -> (manager as LineManager).delete(annotation)
                    is Fill -> (manager as FillManager).delete(annotation)
                }
            }

            // Remove any icon if no other annotations are using it
            annotation.icon()?.let {
                bitmapUsers[it]?.remove(annotation)
                if (bitmapUsers[it]?.isEmpty() == true) {
                    style?.removeImage(it.toString())
                    bitmapUsers.remove(it)
                }
            }

            // Destroy manager if no more annotations with same key remain
            if (!groupAnnotations().containsKey(annotation.key())) {
                managers.remove(annotation.key())?.onDestroy()
            }
        }
    }

    @UiThread
    fun clear() {
        managers.values.forEach {
            it.onDestroy()
        }
        managers.clear()
        annotationList.clear()
    }

    private fun groupAnnotations(): Map<Key, List<KAnnotation<*>>> =
        annotationList.groupBy { it.key() }

    private fun MutableMap<Key, AnnotationManager<*, *>>.getOrCreate(key: Key): AnnotationManager<*, *>? =
        get(key) ?: style?.let { style ->

            val below = managers.keys.firstOrNull { it.z > key.z }?.let { managers[it] }?.layerId

            when (key) {
                is SymbolKey -> SymbolManager(mapView, mapLibreMap, style, below).apply {
                    // Non-collision group symbols do not interfere with each other
                    textAllowOverlap = true
                    iconAllowOverlap = true

                    // Apply NDD properties from key

                    iconTextFit = key.iconFitText.let { fitText ->
                        if (fitText.width && fitText.height) Property.ICON_TEXT_FIT_BOTH
                        else if (fitText.width) Property.ICON_TEXT_FIT_WIDTH
                        else if (fitText.height) Property.ICON_TEXT_FIT_HEIGHT
                        else Property.ICON_TEXT_FIT_NONE
                    }
                    iconTextFitPadding = key.iconFitText.padding.let { padding ->
                        arrayOf(padding.top, padding.right, padding.bottom, padding.left)
                    }

                    iconKeepUpright = key.iconKeepUpright
                    iconPitchAlignment = when (key.iconPitchAlignment) {
                        Alignment.MAP -> Property.ICON_PITCH_ALIGNMENT_MAP
                        Alignment.VIEWPORT -> Property.ICON_PITCH_ALIGNMENT_VIEWPORT
                        null -> Property.ICON_PITCH_ALIGNMENT_AUTO
                    }

                    textPitchAlignment = when (key.textPitchAlignment) {
                        Alignment.MAP -> Property.TEXT_PITCH_ALIGNMENT_MAP
                        Alignment.VIEWPORT -> Property.TEXT_PITCH_ALIGNMENT_VIEWPORT
                        null -> Property.TEXT_PITCH_ALIGNMENT_AUTO
                    }
                    textLineHeight = key.textLineHeight

                }

                is LineKey -> LineManager(mapView, mapLibreMap, style, below).apply {
                    lineCap = when (key.cap) {
                        Cap.BUTT -> Property.LINE_CAP_BUTT
                        Cap.ROUND -> Property.LINE_CAP_ROUND
                        Cap.SQUARE -> Property.LINE_CAP_SQUARE
                    }
                    key.translate?.let { translate ->
                        lineTranslate = arrayOf(translate.offset.x, translate.offset.y)
                        lineTranslateAnchor = when (translate.anchor) {
                            Translate.Anchor.MAP -> Property.LINE_TRANSLATE_ANCHOR_MAP
                            Translate.Anchor.VIEWPORT -> Property.LINE_TRANSLATE_ANCHOR_VIEWPORT
                        }
                    }
                    key.dashArray?.let { dash ->
                        lineDasharray = dash.toTypedArray()
                    }

                }

                is CircleKey -> CircleManager(mapView, mapLibreMap, style, below).apply {
                    key.translate?.let { translate ->
                        circleTranslate = arrayOf(translate.offset.x, translate.offset.y)
                        circleTranslateAnchor = when (translate.anchor) {
                            Translate.Anchor.MAP -> Property.CIRCLE_TRANSLATE_ANCHOR_MAP
                            Translate.Anchor.VIEWPORT -> Property.CIRCLE_TRANSLATE_ANCHOR_VIEWPORT
                        }
                    }

                    circlePitchScale = when (key.pitchScale) {
                        Alignment.MAP -> Property.CIRCLE_PITCH_SCALE_MAP
                        Alignment.VIEWPORT -> Property.CIRCLE_PITCH_SCALE_VIEWPORT
                    }
                    circlePitchAlignment = when (key.pitchAlignment) {
                        Alignment.MAP -> Property.CIRCLE_PITCH_ALIGNMENT_MAP
                        Alignment.VIEWPORT -> Property.CIRCLE_PITCH_ALIGNMENT_VIEWPORT
                    }
                }

                is FillKey -> FillManager(mapView, mapLibreMap, style, below).apply {
                    fillAntialias = key.antialias

                    key.translate?.let { translate ->
                        fillTranslate = arrayOf(translate.offset.x, translate.offset.y)
                        fillTranslateAnchor = when (translate.anchor) {
                            Translate.Anchor.MAP -> Property.FILL_TRANSLATE_ANCHOR_MAP
                            Translate.Anchor.VIEWPORT -> Property.FILL_TRANSLATE_ANCHOR_VIEWPORT
                        }
                    }
                }
            }
        }?.also { put(key, it) }

}
