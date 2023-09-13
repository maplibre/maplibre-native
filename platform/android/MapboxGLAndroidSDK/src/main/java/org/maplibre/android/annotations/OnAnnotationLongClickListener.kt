package org.maplibre.android.annotations

/**
 * Generic interface definition of a callback to be invoked when an annotation has been long clicked.
 *
 * @param T generic parameter extending from Annotation
 */
interface OnAnnotationLongClickListener<T : KAnnotation<*>> {
    /**
     * Called when an annotation has been long clicked
     *
     * @param t the annotation long clicked.
     * @return True if this click should be consumed and not passed further to other listeners
     * registered afterwards, false otherwise.
     */
    fun onAnnotationLongClick(t: T): Boolean
}

/**
 * Interface definition for a callback to be invoked when a symbol has been long clicked.
 */
typealias OnSymbolLongClickListener = OnAnnotationLongClickListener<KSymbol>