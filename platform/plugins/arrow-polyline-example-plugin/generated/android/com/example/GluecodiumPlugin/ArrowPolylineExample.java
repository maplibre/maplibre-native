/*

 *
 */

package com.example.GluecodiumPlugin;

import java.util.List;

/**
 * <p>An example plugin that draws arrow polylines on the map.
 * Takes a list of coordinates and draws a polyline with a chevron arrow head.
 */
public final class ArrowPolylineExample extends MaplibrePlugin {


    public ArrowPolylineExample() {
        this(create(), (Object)null);
        cacheThisInstance();
    }

    /**
     * For internal use only.
     * @hidden
     * @param nativeHandle The SDK nativeHandle instance.
     * @param dummy The SDK dummy instance.
     */
    protected ArrowPolylineExample(final long nativeHandle, final Object dummy) {
        super(nativeHandle, dummy);
    }
    private native void cacheThisInstance();


    private static native long create();

    /**
     * <p>Add an arrow polyline to the map
     * @param coordinates <p>List of LatLng coordinates (at least 2 points)
     * @param config <p>Arrow appearance configuration
     */
    public native void addArrowPolyline(final List<LatLng> coordinates, final ArrowPolylineConfig config);

    /**
     * <p>Remove the current arrow polyline from the map
     */
    public native void removeArrowPolyline();



}

