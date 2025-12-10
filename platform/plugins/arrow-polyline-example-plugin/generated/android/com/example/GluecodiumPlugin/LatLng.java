/*

 *
 */

package com.example.GluecodiumPlugin;


/**
 * <p>A geographic coordinate with latitude and longitude
 */
public final class LatLng {
    public double latitude;
    public double longitude;

    public LatLng(final double latitude, final double longitude) {
        this.latitude = latitude;
        this.longitude = longitude;
    }
    @Override
    public boolean equals(Object obj) {
        if (obj == this) return true;
        if (!(obj instanceof LatLng)) return false;
        final LatLng other = (LatLng) obj;
        return Double.compare(this.latitude, other.latitude) == 0 &&
                Double.compare(this.longitude, other.longitude) == 0;
    }

    @Override
    public int hashCode() {
        int hash = 7;
        hash = 31 * hash + (int) (Double.doubleToLongBits(this.latitude) ^ (Double.doubleToLongBits(this.latitude) >>> 32));
        hash = 31 * hash + (int) (Double.doubleToLongBits(this.longitude) ^ (Double.doubleToLongBits(this.longitude) >>> 32));
        return hash;
    }



}

