package org.maplibre.android.annotations

/**
 * Generic interface definition of a callback to be invoked when an annotation is being dragged.
 *
 * @param T generic parameter extending from Annotation
 */
interface OnAnnotationDragListener<T : KAnnotation<*>> {
    /**
     * Called when an annotation dragging has started.
     *
     * @param annotation the annotation
     */
    fun onAnnotationDragStarted(annotation: T)

    /**
     * Called when an annotation dragging is in progress.
     *
     * @param annotation the annotation
     */
    fun onAnnotationDrag(annotation: T)

    /**
     * Called when an annotation dragging has finished.
     *
     * @param annotation the annotation
     */
    fun onAnnotationDragFinished(annotation: T)
}

/**
 * Interface definition for a callback to be invoked when a symbol is dragged.
 */
typealias OnSymbolDragListener = OnAnnotationDragListener<KSymbol>