package org.maplibre.android.maps;

import java.util.TreeMap;

final public class RouteOptions {
    public int outerColor;
    public int innerColor;
    public double outerWidth = 10;
    public double innerWidth = 6;
    public String layerBefore;
    public boolean useDynamicWidths = false;
    /***
     * outer dynamic width zoom stops is a mapping from zoom levels => route line width for the casing
     */
    public TreeMap<Double, Double> outerDynamicWidthZoomStops = new TreeMap<Double, Double>();

    /***
     * inner dynamic width zoom stops is a mapping from zoom levels => route line width for the
     * blue line
     */
    public TreeMap<Double, Double> innerDynamicWidthZoomStops = new TreeMap<Double, Double>();

}
