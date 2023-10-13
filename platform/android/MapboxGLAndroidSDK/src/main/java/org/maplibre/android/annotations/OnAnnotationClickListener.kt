package org.maplibre.android.annotations

/**
 * Generic interface definition of a callback to be invoked when an annotation has been clicked.
 *
 * @param T generic parameter extending from Annotation
 */
interface OnAnnotationClickListener<T : KAnnotation<*>> {
    /**
     * Called when an annotation has been clicked
     *
     * @param t the annotation clicked.
     * @return True if this click should be consumed and not passed further to other listeners
     * registered afterwards, false otherwise.
     */
    fun onAnnotationClick(t: T): Boolean
}

/**
 * Interface definition for a callback to be invoked when a symbol has been clicked.
 */
typealias OnSymbolClickListener = OnAnnotationClickListener<Symbol>

/**
 * Interface definition for a callback to be invoked when a circle has been clicked.
 */
typealias OnCircleClickListener = OnAnnotationClickListener<Circle>

/**
 * Interface definition for a callback to be invoked when a line has been clicked.
 */
typealias OnLineClickListener = OnAnnotationClickListener<Line>

/**
 * Interface definition for a callback to be invoked when a fill has been clicked.
 */
typealias OnFillClickListener = OnAnnotationClickListener<Fill>