package com.mapbox.mapboxsdk.location.engine;

import android.annotation.SuppressLint;
import android.app.PendingIntent;
import android.content.Context;
import android.location.Location;
import android.os.Looper;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;

import com.google.android.gms.location.FusedLocationProviderClient;
import com.google.android.gms.location.LocationCallback;
import com.google.android.gms.location.LocationRequest;
import com.google.android.gms.location.LocationResult;
import com.google.android.gms.location.LocationServices;
import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;

import java.util.Collections;
import java.util.List;

/**
 * Wraps implementation of Fused Location Provider
 */
class GoogleLocationEngineImpl implements LocationEngineImpl<LocationCallback> {
  private final FusedLocationProviderClient fusedLocationProviderClient;

  @VisibleForTesting
  GoogleLocationEngineImpl(FusedLocationProviderClient fusedLocationProviderClient) {
    this.fusedLocationProviderClient = fusedLocationProviderClient;
  }

  GoogleLocationEngineImpl(@NonNull Context context) {
    this.fusedLocationProviderClient = LocationServices.getFusedLocationProviderClient(context);
  }

  @NonNull
  @Override
  public LocationCallback createListener(LocationEngineCallback<LocationEngineResult> callback) {
    return new GoogleLocationEngineCallbackTransport(callback);
  }

  @SuppressLint("MissingPermission")
  @Override
  public void getLastLocation(@NonNull LocationEngineCallback<LocationEngineResult> callback)
    throws SecurityException {
    GoogleLastLocationEngineCallbackTransport transport =
      new GoogleLastLocationEngineCallbackTransport(callback);
    fusedLocationProviderClient.getLastLocation().addOnSuccessListener(transport).addOnFailureListener(transport);
  }

  @SuppressLint("MissingPermission")
  @Override
  public void requestLocationUpdates(@NonNull LocationEngineRequest request,
                                     @NonNull LocationCallback listener,
                                     @Nullable Looper looper) throws SecurityException {
    fusedLocationProviderClient.requestLocationUpdates(toGMSLocationRequest(request), listener, looper);
  }

  @SuppressLint("MissingPermission")
  @Override
  public void requestLocationUpdates(@NonNull LocationEngineRequest request,
                                     @NonNull PendingIntent pendingIntent) throws SecurityException {
    fusedLocationProviderClient.requestLocationUpdates(toGMSLocationRequest(request), pendingIntent);
  }

  @Override
  public void removeLocationUpdates(@NonNull LocationCallback listener) {
    if (listener != null) {
      fusedLocationProviderClient.removeLocationUpdates(listener);
    }
  }

  @Override
  public void removeLocationUpdates(PendingIntent pendingIntent) {
    if (pendingIntent != null) {
      fusedLocationProviderClient.removeLocationUpdates(pendingIntent);
    }
  }

  private static LocationRequest toGMSLocationRequest(LocationEngineRequest request) {
    LocationRequest locationRequest = new LocationRequest();
    locationRequest.setInterval(request.getInterval());
    locationRequest.setFastestInterval(request.getFastestInterval());
    locationRequest.setSmallestDisplacement(request.getDisplacement());
    locationRequest.setMaxWaitTime(request.getMaxWaitTime());
    locationRequest.setPriority(toGMSLocationPriority(request.getPriority()));
    return locationRequest;
  }

  private static int toGMSLocationPriority(int enginePriority) {
    switch (enginePriority) {
      case LocationEngineRequest.PRIORITY_HIGH_ACCURACY:
        return LocationRequest.PRIORITY_HIGH_ACCURACY;
      case LocationEngineRequest.PRIORITY_BALANCED_POWER_ACCURACY:
        return LocationRequest.PRIORITY_BALANCED_POWER_ACCURACY;
      case LocationEngineRequest.PRIORITY_LOW_POWER:
        return LocationRequest.PRIORITY_LOW_POWER;
      case LocationEngineRequest.PRIORITY_NO_POWER:
      default:
        return LocationRequest.PRIORITY_NO_POWER;
    }
  }

  private static final class GoogleLocationEngineCallbackTransport extends LocationCallback {
    private final LocationEngineCallback<LocationEngineResult> callback;

    GoogleLocationEngineCallbackTransport(LocationEngineCallback<LocationEngineResult> callback) {
      this.callback = callback;
    }

    @Override
    public void onLocationResult(LocationResult locationResult) {
      super.onLocationResult(locationResult);
      List<Location> locations = locationResult.getLocations();
      if (!locations.isEmpty()) {
        callback.onSuccess(LocationEngineResult.create(locations));
      } else {
        callback.onFailure(new Exception("Unavailable location"));
      }
    }
  }

  @VisibleForTesting
  static final class GoogleLastLocationEngineCallbackTransport
    implements OnSuccessListener<Location>, OnFailureListener {
    private final LocationEngineCallback<LocationEngineResult> callback;

    GoogleLastLocationEngineCallbackTransport(LocationEngineCallback<LocationEngineResult> callback) {
      this.callback = callback;
    }

    @Override
    public void onSuccess(Location location) {
      callback.onSuccess(location != null ? LocationEngineResult.create(location) :
        LocationEngineResult.create(Collections.<Location>emptyList()));
    }

    @Override
    public void onFailure(@NonNull Exception e) {
      callback.onFailure(e);
    }
  }
}
