package org.maplibre.android;

public interface XPlatformPluginBridge {
    /**
     * Returns a pointer to a native XPlatformPlugin instance.
     */
    long getNativePlugin();
}
