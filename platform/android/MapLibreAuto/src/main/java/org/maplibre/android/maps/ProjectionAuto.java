/*
 * Copyright (c) 2012-2025 Grab Taxi Holdings PTE LTD (GRAB), All Rights Reserved. NOTICE: All information contained herein is, and remains the property of GRAB. The intellectual and technical concepts contained herein are confidential, proprietary and controlled by GRAB and may be covered by patents, patents in process, and are protected by trade secret or copyright law.
 * You are strictly forbidden to copy, download, store (in any medium), transmit, disseminate, adapt or change this material in any way unless prior written permission is obtained from GRAB. Access to the source code contained herein is hereby forbidden to anyone except current GRAB employees or contractors with binding Confidentiality and Non-disclosure agreements explicitly covering such access.
 *
 * The copyright notice above does not evidence any actual or intended publication or disclosure of this source code, which includes information that is confidential and/or proprietary, and is a trade secret, of GRAB.
 * ANY REPRODUCTION, MODIFICATION, DISTRIBUTION, PUBLIC PERFORMANCE, OR PUBLIC DISPLAY OF OR THROUGH USE OF THIS SOURCE CODE WITHOUT THE EXPRESS WRITTEN CONSENT OF GRAB IS STRICTLY PROHIBITED, AND IN VIOLATION OF APPLICABLE LAWS AND INTERNATIONAL TREATIES. THE RECEIPT OR POSSESSION OF THIS SOURCE CODE AND/OR RELATED INFORMATION DOES NOT CONVEY OR IMPLY ANY RIGHTS TO REPRODUCE, DISCLOSE OR DISTRIBUTE ITS CONTENTS, OR TO MANUFACTURE, USE, OR SELL ANYTHING THAT IT MAY DESCRIBE, IN WHOLE OR IN PART.
 */

package org.maplibre.android.maps;

import android.graphics.PointF;

import androidx.annotation.NonNull;

import org.maplibre.android.constants.GeometryConstants;
import org.maplibre.android.geometry.LatLng;
import org.maplibre.android.geometry.LatLngBounds;
import org.maplibre.android.geometry.VisibleRegion;

import java.util.ArrayList;
import java.util.List;

public class ProjectionAuto extends Projection {

    private final int width, height;

    ProjectionAuto(@NonNull NativeMap nativeMapView, int width, int height) {
        super(nativeMapView, null);
        this.width = width;
        this.height = height;
    }

    @NonNull
    public VisibleRegion getVisibleRegion(boolean ignorePadding) {
        float left;
        float right;
        float top;
        float bottom;

        if (ignorePadding) {
            left = 0;
            right = width;
            top = 0;
            bottom = height;
        } else {
            int[] contentPadding = getContentPadding();
            left = contentPadding[0];
            right = width - contentPadding[2];
            top = contentPadding[1];
            bottom = height - contentPadding[3];
        }

        LatLng center = fromScreenLocation(new PointF(left + (right - left) / 2, top + (bottom - top) / 2));

        LatLng topLeft = fromScreenLocation(new PointF(left, top));
        LatLng topRight = fromScreenLocation(new PointF(right, top));
        LatLng bottomRight = fromScreenLocation(new PointF(right, bottom));
        LatLng bottomLeft = fromScreenLocation(new PointF(left, bottom));

        List<LatLng> latLngs = new ArrayList<>();
        latLngs.add(topRight);
        latLngs.add(bottomRight);
        latLngs.add(bottomLeft);
        latLngs.add(topLeft);

        double maxEastLonSpan = 0;
        double maxWestLonSpan = 0;

        double east = 0;
        double west = 0;
        double north = GeometryConstants.MIN_LATITUDE;
        double south = GeometryConstants.MAX_LATITUDE;

        for (LatLng latLng : latLngs) {
            double bearing = bearing(center, latLng);

            if (bearing >= 0) {
                double span = getLongitudeSpan(latLng.getLongitude(), center.getLongitude());
                if (span > maxEastLonSpan) {
                    maxEastLonSpan = span;
                    east = latLng.getLongitude();
                }
            } else {
                double span = getLongitudeSpan(center.getLongitude(), latLng.getLongitude());
                if (span > maxWestLonSpan) {
                    maxWestLonSpan = span;
                    west = latLng.getLongitude();
                }
            }

            if (north < latLng.getLatitude()) {
                north = latLng.getLatitude();
            }
            if (south > latLng.getLatitude()) {
                south = latLng.getLatitude();
            }
        }

        if (east < west) {
            return new VisibleRegion(topLeft, topRight, bottomLeft, bottomRight,
                    LatLngBounds.from(north, east + GeometryConstants.LONGITUDE_SPAN, south, west));
        }
        return new VisibleRegion(topLeft, topRight, bottomLeft, bottomRight,
                LatLngBounds.from(north, east, south, west));
    }

    @Override
    float getHeight() {
        return height;
    }

    @Override
    float getWidth() {
        return width;
    }
}
