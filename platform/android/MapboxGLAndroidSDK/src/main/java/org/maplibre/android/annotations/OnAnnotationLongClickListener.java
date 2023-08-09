package org.maplibre.android.annotations;

/**
 * Generic interface definition of a callback to be invoked when an annotation has been long clicked.
 *
 * @param <T> generic parameter extending from Annotation
 */
public interface OnAnnotationLongClickListener<T extends AbstractAnnotation> {

    /**
     * Called when an annotation has been long clicked
     *
     * @param t the annotation long clicked.
     * @return True if this click should be consumed and not passed further to other listeners
     * registered afterwards, false otherwise.
     */
    boolean onAnnotationLongClick(T t);
}
