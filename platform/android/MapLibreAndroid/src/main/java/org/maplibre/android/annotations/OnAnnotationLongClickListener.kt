package org.maplibre.android.annotations

/**
 * Called when an annotation has been long-clicked
 *
 * @param T the function gets the annotation that was clicked as an input.
 * @return the function must return `true` if a click should be consumed and not passed further to other listeners
 * registered afterwards, `false` otherwise.
 */
typealias OnAnnotationLongClickListener<T> = (T) -> Boolean

/**
 * Interface definition for a callback to be invoked when a symbol has been long clicked.
 */
typealias OnSymbolLongClickListener = OnAnnotationLongClickListener<Symbol>

/**
 * Interface definition for a callback to be invoked when a circle has been long clicked.
 */
typealias OnCircleLongClickListener = OnAnnotationLongClickListener<Circle>

/**
 * Interface definition for a callback to be invoked when a line has been long clicked.
 */
typealias OnLineLongClickListener = OnAnnotationLongClickListener<Line>

/**
 * Interface definition for a callback to be invoked when a fill has been long clicked.
 */
typealias OnFillLongClickListener = OnAnnotationLongClickListener<Fill>