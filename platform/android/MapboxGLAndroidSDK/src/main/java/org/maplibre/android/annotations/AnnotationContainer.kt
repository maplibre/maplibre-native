package org.maplibre.android.annotations

import androidx.annotation.UiThread
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style

/**
 * Has logic for spawning annotation managers for its collection of annotations.
 */
class KAnnotationContainer(
    private val mapLibreMap: MapLibreMap,
    private val mapView: MapView,
    internal var style: Style?
) {

    private val annotationList: MutableList<KAnnotation<*>> = mutableListOf()
    private val managers: MutableMap<Key, AnnotationManager<*, *>> = mutableMapOf()

    @UiThread
    fun add(annotation: KAnnotation<*>) {
        annotationList.add(annotation)
        addToManager(annotation)
        if (annotation is KSymbol) annotation.icon?.let { style?.addImage(it.image.toString(), it.image) }
    }

    fun updateAll() {
        managers.values.forEach {
            it.onDestroy()
        }
        managers.clear()
        annotationList.forEach {
            addToManager(it)
        }
    }

    fun update(annotation: KAnnotation<*>) {
        managers[annotation.key()]?.updateSource()
        if (annotation is KSymbol) annotation.icon?.let { style?.addImage(it.image.toString(), it.image) }
    }

    private fun addToManager(annotation: KAnnotation<*>) {
        val manager = managers.getOrCreate(annotation.key())
        when (annotation) {
            is KSymbol -> (manager as SymbolManager).add(annotation)
            else -> throw IllegalStateException("For an unknown reason, the compiler started insisting " +
                    "on adding an unreachable else branch. (KAnnotation is a sealed class.)") // TODO
        }
    }

    @UiThread
    fun remove(annotation: KAnnotation<*>) {

        if (annotationList.remove(annotation)) {

            managers[annotation.key()]?.let { manager ->
                when (annotation) {
                    is KSymbol -> (manager as SymbolManager).add(annotation)
                    else -> throw IllegalStateException("For an unknown reason, the compiler started insisting " +
                            "on adding an unreachable else branch. (KAnnotation is a sealed class.)") // TODO
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

    private data class Key(private val type: Class<in KAnnotation<*>>) // TODO will be expanded in the future

    private fun KAnnotation<*>.key() = Key(this.javaClass)

    private fun MutableMap<Key, AnnotationManager<*, *>>.getOrCreate(key: Key) =
        getOrPut(key) {
            SymbolManager(mapView, mapLibreMap, style!!)
        }
}
