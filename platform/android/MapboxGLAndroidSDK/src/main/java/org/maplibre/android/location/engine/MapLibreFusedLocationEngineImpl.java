package org.maplibre.android.location.engine;

import android.annotation.SuppressLint;
import android.app.PendingIntent;
import android.content.Context;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Looper;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import timber.log.Timber;

import static org.maplibre.android.location.engine.Utils.isBetterLocation;

/**
 * MapLibre replacement for Google Play Services Fused Location Client
 * <p>
 * Note: fusion will not work in background mode.
 */
public class MapLibreFusedLocationEngineImpl extends AndroidLocationEngineImpl {
  private static final String TAG = "MapLibreLocationEngine";

  public MapLibreFusedLocationEngineImpl(@NonNull Context context) {
    super(context);
  }

  @NonNull
  @Override
  public LocationListener createListener(LocationEngineCallback<LocationEngineResult> callback) {
    return new MapLibreLocationEngineCallbackTransport(callback);
  }

  @Override
  public void getLastLocation(@NonNull LocationEngineCallback<LocationEngineResult> callback) throws SecurityException {
    Location bestLastLocation = getBestLastLocation();
    if (bestLastLocation != null) {
      callback.onSuccess(LocationEngineResult.create(bestLastLocation));
    } else {
      callback.onFailure(new Exception("Last location unavailable"));
    }
  }

  @SuppressLint("MissingPermission")
  @Override
  public void requestLocationUpdates(@NonNull LocationEngineRequest request,
                                     @NonNull LocationListener listener,
                                     @Nullable Looper looper) throws SecurityException {
    super.requestLocationUpdates(request, listener, looper);

    // Start network provider along with gps
    if (shouldStartNetworkProvider(request.getPriority())) {
      try {
        locationManager.requestLocationUpdates(LocationManager.NETWORK_PROVIDER,
          request.getInterval(), request.getDisplacement(),
          listener, looper);
      } catch (IllegalArgumentException iae) {
        iae.printStackTrace();
      }
    }
  }

  @SuppressLint("MissingPermission")
  @Override
  public void requestLocationUpdates(@NonNull LocationEngineRequest request,
                                     @NonNull PendingIntent pendingIntent) throws SecurityException {
    super.requestLocationUpdates(request, pendingIntent);

    // Start network provider along with gps
    if (shouldStartNetworkProvider(request.getPriority())) {
      try {
        locationManager.requestLocationUpdates(LocationManager.NETWORK_PROVIDER, request.getInterval(),
          request.getDisplacement(), pendingIntent);
      } catch (IllegalArgumentException iae) {
        iae.printStackTrace();
      }
    }
  }

  private Location getBestLastLocation() {
    Location bestLastLocation = null;
    for (String provider : locationManager.getAllProviders()) {
      Location location = getLastLocationFor(provider);
      if (location == null) {
        continue;
      }

      if (isBetterLocation(location, bestLastLocation)) {
        bestLastLocation = location;
      }
    }
    return bestLastLocation;
  }

  private boolean shouldStartNetworkProvider(int priority) {
    return (priority == LocationEngineRequest.PRIORITY_HIGH_ACCURACY
      || priority == LocationEngineRequest.PRIORITY_BALANCED_POWER_ACCURACY)
      && currentProvider.equals(LocationManager.GPS_PROVIDER);
  }

  private static final class MapLibreLocationEngineCallbackTransport implements LocationListener {
    private final LocationEngineCallback<LocationEngineResult> callback;
    private Location currentBestLocation;

    MapLibreLocationEngineCallbackTransport(LocationEngineCallback<LocationEngineResult> callback) {
      this.callback = callback;
    }

    @Override
    public void onLocationChanged(Location location) {
      if (isBetterLocation(location, currentBestLocation)) {
        currentBestLocation = location;
      }

      if (callback != null) {
        callback.onSuccess(LocationEngineResult.create(currentBestLocation));
      }
    }

    @Override
    public void onStatusChanged(String provider, int status, Bundle extras) {
      Timber.d( "onStatusChanged: " + provider);
    }

    @Override
    public void onProviderEnabled(String provider) {
      Timber.d( "onProviderEnabled: " + provider);
    }

    @Override
    public void onProviderDisabled(String provider) {
      Timber.d("onProviderDisabled: " + provider);
    }
  }
}
