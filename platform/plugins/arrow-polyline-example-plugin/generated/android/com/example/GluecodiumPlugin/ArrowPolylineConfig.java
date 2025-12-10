/*

 *
 */

package com.example.GluecodiumPlugin;


/**
 * <p>Configuration for arrow polyline appearance
 */
public final class ArrowPolylineConfig {
    /**
     * <p>Length of the arrow head in pixels
     */
    public double headLength;
    /**
     * <p>Angle of the arrow head in degrees
     */
    public double headAngle;
    /**
     * <p>Color of the arrow line as hex string (e.g., &quot;#FF0000&quot;)
     */
    public String lineColor;
    /**
     * <p>Width of the arrow line in pixels
     */
    public double lineWidth;

    public ArrowPolylineConfig() {
        this.headLength = 50.0;
        this.headAngle = 30.0;
        this.lineColor = "#FF0000";
        this.lineWidth = 3.0;
    }


}
