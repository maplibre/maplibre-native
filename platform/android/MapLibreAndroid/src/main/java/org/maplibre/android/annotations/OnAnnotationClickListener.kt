package org.maplibre.android.annotations


/**
 * Called when an annotation has been clicked
 *
 * @param T the function gets the annotation that was clicked as an input.
 * @return the function must return `true` if a click should be consumed and not passed further to other listeners
 * registered afterwards, `false` otherwise.
 */
typealias OnAnnotationClickListener<T> = (T) -> Boolean

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