package org.maplibre.maplibrepluginsexamplelibrary;

public class NativeLib {

    public native String stringFromJNI();

    public native void registerPlugins();

    static {
        System.loadLibrary("maplibre");
        System.loadLibrary("maplibrepluginsexamplelibrary");
    }
}
