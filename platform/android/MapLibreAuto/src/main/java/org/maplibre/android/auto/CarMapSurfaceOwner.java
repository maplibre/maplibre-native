package org.maplibre.android.auto;

import android.graphics.Rect;

import androidx.annotation.NonNull;
import androidx.car.app.AppManager;
import androidx.car.app.CarContext;
import androidx.car.app.SurfaceCallback;
import androidx.car.app.SurfaceContainer;
import androidx.car.app.annotations.ExperimentalCarApi;

import org.maplibre.android.maps.MapLibreMapOptions;
import org.maplibre.android.maps.MapSurface;
import org.maplibre.android.maps.OnMapReadyCallback;


public class CarMapSurfaceOwner implements SurfaceCallback {

    private static final String TAG = "CarMapSurfaceOwner";

    private final MapLibreMapOptions grabMapOptions;

    private final CarContext carContext;

    private MapSurface mapSurface;

    private final OnMapReadyCallback callback;

    public CarMapSurfaceOwner(CarContext carContext,
                              MapLibreMapOptions grabMapOptions,
                              OnMapReadyCallback callback
    ) {
        this.carContext = carContext;
        this.grabMapOptions = grabMapOptions;
        this.callback = callback;
        this.carContext.getCarService(AppManager.class)
                .setSurfaceCallback(this);
    }

    @Override
    public void onSurfaceAvailable(SurfaceContainer surfaceContainer) {
        if (surfaceContainer.getSurface() != null) {
            MapSurface mapSurface = new MapSurface(carContext,
                    grabMapOptions,
                    surfaceContainer.getSurface(),
                    surfaceContainer.getWidth(),
                    surfaceContainer.getHeight());
            mapSurface.getMapAsync(callback);
            mapSurface.onStart();

            if (CarMapSurfaceOwner.this.mapSurface != null) {
                mapSurface.onStop();
                mapSurface.onSurfaceDestroyed();
                mapSurface.onDestroy();
            }

            CarMapSurfaceOwner.this.mapSurface = mapSurface;
        }
    }

    @Override
    public void onSurfaceDestroyed(SurfaceContainer surfaceContainer) {
        MapSurface detachSurface = this.mapSurface;
        detachSurface.onStop();
        detachSurface.onSurfaceDestroyed();
        detachSurface.onDestroy();
        mapSurface = null;
    }

    @Override
    public void onVisibleAreaChanged(@NonNull Rect visibleArea) {
        SurfaceCallback.super.onVisibleAreaChanged(visibleArea);
        if (mapSurface != null) {
            mapSurface.onVisibleAreaChanged(visibleArea);
        }
    }

    @Override
    public void onStableAreaChanged(@NonNull Rect stableArea) {
        SurfaceCallback.super.onStableAreaChanged(stableArea);
    }


    @Override
    public void onScroll(float distanceX, float distanceY) {
        if (mapSurface != null) {
            mapSurface.onScroll(distanceX, distanceY);
        }
    }

    @Override
    public void onFling(float velocityX, float velocityY) {
        if (mapSurface != null) {
            mapSurface.onFling(velocityX, velocityY);
        }
    }

    @Override
    public void onScale(float focusX, float focusY, float scaleFactor) {
        if (mapSurface != null) {
            mapSurface.onScale(focusX, focusY, scaleFactor);
        }
    }

    @ExperimentalCarApi
    @Override
    public void onClick(float x, float y) {
        if (mapSurface != null) {
            mapSurface.onClick(x, y);
        }

    }

    public MapSurface getMapSurface() {
        return mapSurface;
    }
}
