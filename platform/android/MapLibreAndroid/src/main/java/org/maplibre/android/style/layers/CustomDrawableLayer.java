package org.maplibre.android.style.layers;

import androidx.annotation.Keep;

/**
 * Custom drawable layer that supports cross-backend rendering
 * via the CustomDrawableLayerHost interface.
 * <p>
 * Unlike {@link CustomLayer}, this layer works with OpenGL, Vulkan,
 * and other rendering backends through a high-level drawable API.
 * </p>
 */
public class CustomDrawableLayer extends Layer {

    public CustomDrawableLayer(String id, long host) {
        initialize(id, host);
    }

    @Keep
    CustomDrawableLayer(long nativePtr) {
        super(nativePtr);
    }

    @Keep
    protected native void initialize(String id, long host);

    @Override
    @Keep
    protected native void finalize() throws Throwable;
}
