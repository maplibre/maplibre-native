package org.maplibre.android.location.engine;

import android.annotation.SuppressLint;
import android.app.PendingIntent;
import android.content.Context;
import android.location.Criteria;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;

/**
 * A location engine that uses core android.location and has no external dependencies
 * https://developer.android.com/guide/topics/location/strategies.html
 */
public class AndroidLocationEngineImpl implements LocationEngineImpl<LocationListener> {
  private static final String TAG = "AndroidLocationEngine";
  final LocationManager locationManager;

  String currentProvider = LocationManager.PASSIVE_PROVIDER;

  public AndroidLocationEngineImpl(@NonNull Context context) {
    locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
  }

  @NonNull
  @Override
  public LocationListener createListener(LocationEngineCallback<LocationEngineResult> callback) {
    return new AndroidLocationEngineCallbackTransport(callback);
  }

  @Override
  public void getLastLocation(@NonNull LocationEngineCallback<LocationEngineResult> callback)
    throws SecurityException {
    Location lastLocation = getLastLocationFor(currentProvider);
    if (lastLocation != null) {
      callback.onSuccess(LocationEngineResult.create(lastLocation));
      return;
    }

    for (String provider : locationManager.getAllProviders()) {
      lastLocation = getLastLocationFor(provider);
      if (lastLocation != null) {
        callback.onSuccess(LocationEngineResult.create(lastLocation));
        return;
      }
    }
    callback.onFailure(new Exception("Last location unavailable"));
  }

  @SuppressLint("MissingPermission")
  Location getLastLocationFor(String provider) throws SecurityException {
    Location location = null;
    try {
      location = locationManager.getLastKnownLocation(provider);
    } catch (IllegalArgumentException iae) {
      Log.e(TAG, iae.toString());
    }
    return location;
  }

  @SuppressLint("MissingPermission")
  @Override
  public void requestLocationUpdates(@NonNull LocationEngineRequest request,
                                     @NonNull LocationListener listener,
                                     @Nullable Looper looper) throws SecurityException {
    // Pick best provider only if user has not explicitly chosen passive mode
    currentProvider = getBestProvider(request.getPriority());
    locationManager.requestLocationUpdates(currentProvider, request.getInterval(), request.getDisplacement(),
      listener, looper);
  }

  @SuppressLint("MissingPermission")
  @Override
  public void requestLocationUpdates(@NonNull LocationEngineRequest request,
                                     @NonNull PendingIntent pendingIntent) throws SecurityException {
    // Pick best provider only if user has not explicitly chosen passive mode
    currentProvider = getBestProvider(request.getPriority());
    locationManager.requestLocationUpdates(currentProvider, request.getInterval(),
      request.getDisplacement(), pendingIntent);
  }

  @SuppressLint("MissingPermission")
  @Override
  public void removeLocationUpdates(@NonNull LocationListener listener) {
    if (listener != null) {
      locationManager.removeUpdates(listener);
    }
  }

  @Override
  public void removeLocationUpdates(PendingIntent pendingIntent) {
    if (pendingIntent != null) {
      locationManager.removeUpdates(pendingIntent);
    }
  }

  private String getBestProvider(int priority) {
    String provider = null;
    // Pick best provider only if user has not explicitly chosen passive mode
    if (priority != LocationEngineRequest.PRIORITY_NO_POWER) {
      provider = locationManager.getBestProvider(getCriteria(priority), true);
    }
    return provider != null ? provider : LocationManager.PASSIVE_PROVIDER;
  }

  @VisibleForTesting
  static Criteria getCriteria(int priority) {
    Criteria criteria = new Criteria();
    criteria.setAccuracy(priorityToAccuracy(priority));
    criteria.setCostAllowed(true);
    criteria.setPowerRequirement(priorityToPowerRequirement(priority));
    return criteria;
  }

  private static int priorityToAccuracy(int priority) {
    switch (priority) {
      case LocationEngineRequest.PRIORITY_HIGH_ACCURACY:
      case LocationEngineRequest.PRIORITY_BALANCED_POWER_ACCURACY:
        return Criteria.ACCURACY_FINE;
      case LocationEngineRequest.PRIORITY_LOW_POWER:
      case LocationEngineRequest.PRIORITY_NO_POWER:
      default:
        return Criteria.ACCURACY_COARSE;
    }
  }

  private static int priorityToPowerRequirement(int priority) {
    switch (priority) {
      case LocationEngineRequest.PRIORITY_HIGH_ACCURACY:
        return Criteria.POWER_HIGH;
      case LocationEngineRequest.PRIORITY_BALANCED_POWER_ACCURACY:
        return Criteria.POWER_MEDIUM;
      case LocationEngineRequest.PRIORITY_LOW_POWER:
      case LocationEngineRequest.PRIORITY_NO_POWER:
      default:
        return Criteria.POWER_LOW;
    }
  }

  @VisibleForTesting
  static final class AndroidLocationEngineCallbackTransport implements LocationListener {
    private final LocationEngineCallback<LocationEngineResult> callback;

    AndroidLocationEngineCallbackTransport(LocationEngineCallback<LocationEngineResult> callback) {
      this.callback = callback;
    }

    @Override
    public void onLocationChanged(Location location) {
      callback.onSuccess(LocationEngineResult.create(location));
    }

    @Override
    public void onStatusChanged(String s, int i, Bundle bundle) {
      // noop
    }

    @Override
    public void onProviderEnabled(String s) {
      // noop
    }

    @Override
    public void onProviderDisabled(String s) {
      callback.onFailure(new Exception("Current provider disabled"));
    }
  }
}
