package org.maplibre.android.annotations

import android.graphics.Paint.Cap
import androidx.annotation.UiThread
import org.maplibre.android.annotations.data.Alignment
import org.maplibre.android.annotations.data.Icon
import org.maplibre.android.annotations.data.Translate
import org.maplibre.android.annotations.data.toArray
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
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

    @JvmName("setStyle")
    internal fun setStyle(style: Style) {
        this.style = style
        updateAll()
    }

    @UiThread
    fun add(annotation: KAnnotation<*>) {
        annotationList.add(annotation)
        addToManager(annotation)
        if (annotation is Symbol) annotation.icon?.let { style?.addImage(it.image.toString(), it.image) }
        if (annotation is Line) annotation.pattern?.let { style?.addImage(it.toString(), it) }
        if (annotation is Fill) annotation.pattern?.let { style?.addImage(it.toString(), it) }
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
        get(key) ?: style?.let {
            when (key) {
                is SymbolKey -> SymbolManager(mapView, mapLibreMap, it).apply {
                    // Non-collision group symbols do not interfere with each other
                    textAllowOverlap = true
                    iconAllowOverlap = true

                    // Apply NDD properties from key

                    iconTextFit = key.iconFitText.let { fitText ->
                        if (fitText.width && fitText.height) "both"
                        else if (fitText.width) "width"
                        else if (fitText.height) "height"
                        else "none"
                    }
                    iconTextFitPadding = key.iconFitText.padding.let { padding ->
                        arrayOf(padding.top, padding.right, padding.bottom, padding.left)
                    }

                    iconKeepUpright = key.iconKeepUpright
                    iconPitchAlignment = when (key.iconPitchAlignment) {
                        Alignment.MAP -> "map"
                        Alignment.VIEWPORT -> "viewport"
                        null -> "auto"
                    }

                    textPitchAlignment = when (key.textPitchAlignment) {
                        Alignment.MAP -> "map"
                        Alignment.VIEWPORT -> "viewport"
                        null -> "auto"
                    }
                    textLineHeight = key.textLineHeight

                }

                is LineKey -> LineManager(mapView, mapLibreMap, it).apply {
                    lineCap = when (key.cap) {
                        Cap.BUTT -> "butt"
                        Cap.ROUND -> "round"
                        Cap.SQUARE -> "square"
                    }
                    key.translate?.let { translate ->
                        lineTranslate = arrayOf(translate.offset.x, translate.offset.y)
                        lineTranslateAnchor = when (translate.anchor) {
                            Translate.Anchor.MAP -> "map"
                            Translate.Anchor.VIEWPORT -> "viewport"
                        }
                    }
                    key.dashArray?.let { dash ->
                        lineDasharray = dash
                    }

                }

                is CircleKey -> CircleManager(mapView, mapLibreMap, it).apply {
                    key.translate?.let { translate ->
                        circleTranslate = arrayOf(translate.offset.x, translate.offset.y)
                        circleTranslateAnchor = when (translate.anchor) {
                            Translate.Anchor.MAP -> "map"
                            Translate.Anchor.VIEWPORT -> "viewport"
                        }
                    }

                    circlePitchScale = when (key.pitchScale) {
                        Alignment.MAP -> "map"
                        Alignment.VIEWPORT -> "viewport"
                    }
                    circlePitchAlignment = when (key.pitchAlignment) {
                        Alignment.MAP -> "map"
                        Alignment.VIEWPORT -> "viewport"
                    }
                }

                is FillKey -> FillManager(mapView, mapLibreMap, it).apply {
                    fillAntialias = key.antialias

                    key.translate?.let { translate ->
                        fillTranslate = arrayOf(translate.offset.x, translate.offset.y)
                        fillTranslateAnchor = when (translate.anchor) {
                            Translate.Anchor.MAP -> "map"
                            Translate.Anchor.VIEWPORT -> "viewport"
                        }
                    }
                }
            }
        }?.also { put(key, it) }

}
