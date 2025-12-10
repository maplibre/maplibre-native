/*

 *
 */

package com.example.GluecodiumPlugin;

import com.example.NativeBase;

/**
 * <p>Base class for gluecodium-based maplibre plugins
 * (In a full integration, this would be in a shared package)
 */
public class MaplibrePlugin extends NativeBase {


    public MaplibrePlugin() {
        this(create(), (Object)null);
        cacheThisInstance();
    }

    /**
     * For internal use only.
     * @hidden
     * @param nativeHandle The SDK nativeHandle instance.
     * @param dummy The SDK dummy instance.
     */
    protected MaplibrePlugin(final long nativeHandle, final Object dummy) {
        super(nativeHandle, new Disposer() {
            @Override
            public void disposeNative(long handle) {
                disposeNativeHandle(handle);
            }
        });
    }

    private static native void disposeNativeHandle(long nativeHandle);
    private native void cacheThisInstance();


    private static native long create();

    /**
     *
     * @return <p>Pointer to the underlying C++ class that implements XPlatformPlugin
     */
    public native long getPtr();



}

