package org.maplibre.android.annotations

import androidx.annotation.UiThread
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

            managers[annotation.key()]?.let { manager ->
                when (annotation) {
                    is Symbol -> (manager as SymbolManager).delete(annotation)
                    is Circle -> (manager as CircleManager).delete(annotation)
                    is Line -> (manager as LineManager).delete(annotation)
                    is Fill -> (manager as FillManager).delete(annotation)
                }
            }

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

    private data class Key(val type: KClass<out KAnnotation<*>>) // TODO will be expanded in the future

    private fun KAnnotation<*>.key() = Key(this::class)

    private fun MutableMap<Key, AnnotationManager<*, *>>.getOrCreate(key: Key): AnnotationManager<*, *>? =
        get(key) ?: style?.let {
            when (key.type) {
                Symbol::class -> SymbolManager(mapView, mapLibreMap, it)
                Circle::class -> CircleManager(mapView, mapLibreMap, it)
                Line::class -> LineManager(mapView, mapLibreMap, it)
                Fill::class -> FillManager(mapView, mapLibreMap, it)
                else -> throw IllegalArgumentException(
                    "Impossible key! This should never occur because KAnnotation is a sealed class."
                )
            }
        }?.also { put(key, it) }

}
