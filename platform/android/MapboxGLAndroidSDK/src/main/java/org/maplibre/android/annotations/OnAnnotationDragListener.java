package org.maplibre.android.annotations;

/**
 * Generic interface definition of a callback to be invoked when an annotation is being dragged.
 *
 * @param <T> generic parameter extending from Annotation
 */
public interface OnAnnotationDragListener<T extends AbstractAnnotation> {

    /**
     * Called when an annotation dragging has started.
     *
     * @param annotation the annotation
     */
    void onAnnotationDragStarted(T annotation);

    /**
     * Called when an annotation dragging is in progress.
     *
     * @param annotation the annotation
     */
    void onAnnotationDrag(T annotation);

    /**
     * Called when an annotation dragging has finished.
     *
     * @param annotation the annotation
     */
    void onAnnotationDragFinished(T annotation);
}