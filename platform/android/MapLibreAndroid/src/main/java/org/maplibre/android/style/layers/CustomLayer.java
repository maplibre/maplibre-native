package org.maplibre.android.style.layers;

import androidx.annotation.Keep;

/**
 * A style layer that is rendered by native drawing code that you provide.
 * <p>
 * The {@code host} passed to the constructor is a pointer to a native
 * {@code mln::style::CustomLayerHost} implementation, created in your own JNI code.
 * The layer takes ownership of the host. Its callbacks run on the render thread and
 * issue draw calls directly with the graphics API of the active rendering backend
 * (OpenGL ES or Vulkan).
 * </p>
 * <p>
 * <strong>This is an advanced feature. Use it at your own risk.</strong>
 * Bugs in your drawing code can break rendering or crash the app, and keeping it
 * working with future versions of MapLibre Native is up to you.
 * </p>
 */
public class CustomLayer extends Layer {

  public CustomLayer(String id,
                     long host) {
    initialize(id, host);
  }

  /**
   * Triggers map re-paint.
   *
   * @deprecated Use {@link MapLibreMap#triggerRepaint()} instead.
   */
  @Deprecated
  @Keep
  public void update() {
  }

  @Keep
  CustomLayer(long nativePtr) {
    super(nativePtr);
  }

  @Keep
  protected native void initialize(String id, long host);

  @Override
  @Keep
  protected native void finalize() throws Throwable;

}
