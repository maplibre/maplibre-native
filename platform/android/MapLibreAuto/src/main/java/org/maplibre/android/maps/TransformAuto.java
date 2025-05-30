package org.maplibre.android.maps;

import android.os.Handler;
import android.os.Looper;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.UiThread;

import org.maplibre.android.camera.CameraPosition;
import org.maplibre.android.camera.CameraUpdate;
import org.maplibre.android.camera.CameraUpdateFactory;

public final class TransformAuto extends Transform {

    private static final String TAG = "GrabMap-Auto-Transform";

    private final NativeMap nativeMap;
    @Nullable
    private MapView mapView;
    @Nullable
    private MapSurface mapSurface;

    private final Handler handler = new Handler(Looper.getMainLooper());

    @Nullable
    private CameraPosition cameraPosition;
    @Nullable
    private MapLibreMap.CancelableCallback cameraCancelableCallback;
    private final CameraChangeDispatcher cameraChangeDispatcher;

    private final MapView.OnCameraDidChangeListener moveByChangeListener = new MapView.OnCameraDidChangeListener() {
        @Override
        public void onCameraDidChange(boolean animated) {
            if (animated) {
                cameraChangeDispatcher.onCameraIdle();
                removeOnCameraDidChangeListener(this);
            }
        }
    };

    TransformAuto(@NonNull MapView mapView, NativeMap nativeMap, CameraChangeDispatcher cameraChangeDispatcher) {
        super(mapView, nativeMap, cameraChangeDispatcher);
        this.mapView = mapView;
        this.nativeMap = nativeMap;
        this.cameraChangeDispatcher = cameraChangeDispatcher;
    }

    TransformAuto(@NonNull MapSurface mapSurface, NativeMap nativeMap, CameraChangeDispatcher cameraChangeDispatcher) {
        super(null, nativeMap, cameraChangeDispatcher);
        this.mapSurface = mapSurface;
        this.nativeMap = nativeMap;
        this.cameraChangeDispatcher = cameraChangeDispatcher;
    }

    void initialise(@NonNull MapLibreMap maplibreMap, @NonNull MapLibreMapOptions options) {
        CameraPosition position = options.getCamera();
        if (position != null && !position.equals(CameraPosition.DEFAULT)) {
            moveCamera(maplibreMap, CameraUpdateFactory.newCameraPosition(position), null);
        }
        setMinZoom(options.getMinZoomPreference());
        setMaxZoom(options.getMaxZoomPreference());
        setMinPitch(options.getMinPitchPreference());
        setMaxPitch(options.getMaxPitchPreference());
    }

    //
    // Camera API
    //

    @Nullable
    @UiThread
    public CameraPosition getCameraPosition() {
        if (cameraPosition == null) {
            cameraPosition = invalidateCameraPosition();
        }
        return cameraPosition;
    }

    @Override
    public void onCameraDidChange(boolean animated) {
        if (animated) {
            invalidateCameraPosition();
            if (cameraCancelableCallback != null) {
                final MapLibreMap.CancelableCallback callback = cameraCancelableCallback;

                // nullification has to happen before Handler#post, see https://github.com/robolectric/robolectric/issues/1306
                cameraCancelableCallback = null;

                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        callback.onFinish();
                    }
                });
            }
            cameraChangeDispatcher.onCameraIdle();
            removeOnCameraDidChangeListener(this);
        }
    }

    /**
     * Internal use.
     */
    @UiThread
    public void moveCamera(@NonNull MapLibreMap maplibreMap, CameraUpdate update,
                           @Nullable final MapLibreMap.CancelableCallback callback) {
        CameraPosition cameraPosition = update.getCameraPosition(maplibreMap);
        if (isValidCameraPosition(cameraPosition)) {
            cancelTransitions();
            cameraChangeDispatcher.onCameraMoveStarted(MapLibreMap.OnCameraMoveStartedListener.REASON_API_ANIMATION);
            nativeMap.jumpTo(cameraPosition.target, cameraPosition.zoom, cameraPosition.tilt, cameraPosition.bearing,
                    cameraPosition.padding);
            invalidateCameraPosition();
            cameraChangeDispatcher.onCameraIdle();
            handler.post(new Runnable() {
                @Override
                public void run() {
                    if (callback != null) {
                        callback.onFinish();
                    }
                }
            });
        } else if (callback != null) {
            callback.onFinish();
        }
    }

    @UiThread
    void easeCamera(@NonNull MapLibreMap maplibreMap, CameraUpdate update, int durationMs,
                    boolean easingInterpolator,
                    @Nullable final MapLibreMap.CancelableCallback callback) {
        CameraPosition cameraPosition = update.getCameraPosition(maplibreMap);
        if (isValidCameraPosition(cameraPosition)) {
            cancelTransitions();
            cameraChangeDispatcher.onCameraMoveStarted(MapLibreMap.OnCameraMoveStartedListener.REASON_API_ANIMATION);

            if (callback != null) {
                cameraCancelableCallback = callback;
            }
            addOnCameraDidChangeListener(this);
            nativeMap.easeTo(cameraPosition.target, cameraPosition.zoom, cameraPosition.bearing, cameraPosition.tilt,
                    cameraPosition.padding, durationMs, easingInterpolator);
        } else if (callback != null) {
            callback.onFinish();
        }
    }

    /**
     * Internal use.
     */
    @UiThread
    public void animateCamera(@NonNull MapLibreMap maplibreMap, CameraUpdate update, int durationMs,
                              @Nullable final MapLibreMap.CancelableCallback callback) {
        CameraPosition cameraPosition = update.getCameraPosition(maplibreMap);
        if (isValidCameraPosition(cameraPosition)) {
            cancelTransitions();
            cameraChangeDispatcher.onCameraMoveStarted(MapLibreMap.OnCameraMoveStartedListener.REASON_API_ANIMATION);

            if (callback != null) {
                cameraCancelableCallback = callback;
            }
            addOnCameraDidChangeListener(this);
            nativeMap.flyTo(cameraPosition.target, cameraPosition.zoom, cameraPosition.bearing,
                    cameraPosition.tilt, cameraPosition.padding, durationMs);
        } else if (callback != null) {
            callback.onFinish();
        }
    }

    void moveBy(double offsetX, double offsetY, long duration) {
        if (duration > 0) {
            addOnCameraDidChangeListener(moveByChangeListener);
        }
        nativeMap.moveBy(offsetX, offsetY, duration);
    }

    private boolean isValidCameraPosition(@Nullable CameraPosition cameraPosition) {
        return cameraPosition != null && !cameraPosition.equals(this.cameraPosition);
    }

    private void addOnCameraDidChangeListener(@NonNull MapView.OnCameraDidChangeListener listener) {
        if (mapSurface != null) {
            mapSurface.addOnCameraDidChangeListener(listener);
        } else if (mapView != null) {
            mapView.addOnCameraDidChangeListener(listener);
        }
    }

    private void removeOnCameraDidChangeListener(@NonNull MapView.OnCameraDidChangeListener listener) {
        if (mapSurface != null) {
            mapSurface.removeOnCameraDidChangeListener(listener);
        } else if (mapView != null) {
            mapView.removeOnCameraDidChangeListener(listener);
        }
    }
}
