package org.maplibre.android.annotations;

/**
 * Generic interface definition of a callback to be invoked when an annotation has been clicked.
 *
 * @param <T> generic parameter extending from Annotation
 */
public interface OnAnnotationClickListener<T extends AbstractAnnotation> {

    /**
     * Called when an annotation has been clicked
     *
     * @param t the annotation clicked.
     * @return True if this click should be consumed and not passed further to other listeners
     * registered afterwards, false otherwise.
     */
    boolean onAnnotationClick(T t);
}
