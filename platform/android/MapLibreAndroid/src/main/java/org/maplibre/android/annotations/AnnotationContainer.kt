package org.maplibre.android.annotations

import android.graphics.Bitmap
import android.graphics.Paint.Cap
import androidx.annotation.UiThread
import androidx.annotation.VisibleForTesting
import org.maplibre.android.annotations.data.Alignment
import org.maplibre.android.annotations.data.Translate
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.style.layers.CircleLayer
import org.maplibre.android.style.layers.FillLayer
import org.maplibre.android.style.layers.LineLayer
import org.maplibre.android.style.layers.Property
import org.maplibre.android.style.layers.SymbolLayer

/**
 * Has logic for spawning annotation managers for its collection of annotations.
 */
internal class KAnnotationContainer
@JvmOverloads constructor(
    private val mapLibreMap: MapLibreMap,
    private val mapView: MapView,
    private var style: Style?,
    private val draggableAnnotationController: DraggableAnnotationController = DraggableAnnotationController.getInstance(
        mapView, mapLibreMap
    ),
    private val symbolElementProviderGenerator: () -> CoreElementProvider<SymbolLayer> = { SymbolElementProvider() },
    private val lineElementProviderGenerator: () -> CoreElementProvider<LineLayer> = { LineElementProvider() },
    private val circleElementProviderGenerator: () -> CoreElementProvider<CircleLayer> = { CircleElementProvider() },
    private val fillElementProviderGenerator: () -> CoreElementProvider<FillLayer> = { FillElementProvider() }
) {

    private val annotationList: MutableList<KAnnotation<*>> = mutableListOf()
    private val managers: MutableMap<Key, AnnotationManager<*, *>> = mutableMapOf()
    private val bitmapUsers: MutableMap<Bitmap, MutableSet<KAnnotation<*>>> = mutableMapOf()

    private val collisionGroups: MutableSet<CollisionGroup> = mutableSetOf()

    @JvmName("setStyle")
    internal fun setStyle(style: Style?) {
        this.style = style
        updateAll()
    }

    @UiThread
    fun add(annotation: KAnnotation<*>) {
        annotationList.add(annotation)
        annotation.icon()?.add(forAnnotation = annotation)
        addToManager(annotation)
    }

    @UiThread
    fun add(collisionGroup: CollisionGroup) {

        collisionGroups.add(collisionGroup)
        collisionGroup.symbols.forEach {
            it.icon()?.add(forAnnotation = it)
        }

        collisionGroup.key().let { key ->
            (managers.getOrCreate(key) as SymbolManager).apply {

                deleteAll()
                addAll(collisionGroup.symbols)

                collisionGroup.manager = this
            }
        }
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

    fun Bitmap.remove(forAnnotation: KAnnotation<*>) {
        bitmapUsers[this]?.remove(forAnnotation)
        if (bitmapUsers[this]?.isEmpty() == true) {
            style?.removeImage(this.toString())
            bitmapUsers.remove(this)
        }
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
    fun remove(collisionGroup: CollisionGroup) {
        if (collisionGroups.remove(collisionGroup)) {

            managers.keys.filter { it is CollisionGroupKey && it.collisionGroup == collisionGroup }.forEach {
                managers[it]?.onDestroy()
                managers.remove(it)
            }

            collisionGroup.symbols.forEach {
                it.icon()?.remove(forAnnotation = it)
            }
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
            annotation.icon()?.remove(annotation)

            // Destroy manager if no more annotations with same key remain
            if (!groupAnnotations().containsKey(annotation.key())) {
                managers.remove(annotation.key())?.onDestroy()
            }
        }
    }

    @UiThread
    fun clear() {
        collisionGroups.forEach {
            it.manager = null
        }
        collisionGroups.clear()
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
                is SymbolKey, is CollisionGroupKey -> SymbolManager(
                    mapView,
                    mapLibreMap,
                    style,
                    belowLayerId = below,
                    draggableAnnotationController = draggableAnnotationController,
                    coreElementProvider = symbolElementProviderGenerator()
                ).apply {

                    if (key is CollisionGroupKey) {
                        symbolSpacing = key.collisionGroup.symbolSpacing
                        symbolAvoidEdges = key.collisionGroup.symbolAvoidEdges
                        iconAllowOverlap = key.collisionGroup.iconAllowOverlap
                        iconIgnorePlacement = key.collisionGroup.iconIgnorePlacement
                        iconOptional = key.collisionGroup.iconOptional
                        iconPadding = key.collisionGroup.iconPadding
                        textPadding = key.collisionGroup.textPadding
                        textAllowOverlap = key.collisionGroup.textAllowOverlap
                        textIgnorePlacement = key.collisionGroup.textIgnorePlacement
                        textOptional = key.collisionGroup.textOptional
                        key.collisionGroup.textVariableAnchor?.map {
                            it.toString()
                        }?.toTypedArray().let {
                            textVariableAnchor = it
                        }
                    } else {
                        // Non-collision group symbols do not interfere with each other
                        // Collision group properties are handled in their `addOrUpdate` method
                        textAllowOverlap = true
                        iconAllowOverlap = true
                    }

                    // Apply NDD properties from symbol key
                    when (key) {
                        is CollisionGroupKey -> {
                            if (key.collisionGroup.symbols.isEmpty()) return@apply

                            key.collisionGroup.symbols[0].key() as SymbolKey
                        }
                        is SymbolKey -> key
                        else -> throw IllegalStateException()
                    }.applyProperties(this)


                }

                is LineKey -> LineManager(
                    mapView = mapView,
                    maplibreMap = mapLibreMap,
                    style = style,
                    belowLayerId = below,
                    draggableAnnotationController = draggableAnnotationController,
                    coreElementProvider = lineElementProviderGenerator()
                ).apply {
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

                is CircleKey -> CircleManager(
                    mapView = mapView,
                    maplibreMap = mapLibreMap,
                    style = style,
                    belowLayerId = below,
                    draggableAnnotationController = draggableAnnotationController,
                    coreElementProvider = circleElementProviderGenerator()
                ).apply {
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

                is FillKey -> FillManager(
                    mapView = mapView,
                    maplibreMap = mapLibreMap,
                    style = style,
                    belowLayerId = below,
                    draggableAnnotationController = draggableAnnotationController,
                    coreElementProvider = fillElementProviderGenerator()
                ).apply {
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

    @VisibleForTesting
    internal val size get() = annotationList.size + collisionGroups.sumOf { it.symbols.size }

    @VisibleForTesting
    internal val managerCount get() = managers.size
}
