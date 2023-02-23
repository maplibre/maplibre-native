package com.mapbox.mapboxsdk.location.engine;

import android.app.PendingIntent;

import com.google.android.gms.location.FusedLocationProviderClient;
import com.google.android.gms.location.LocationCallback;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

@RunWith(MockitoJUnitRunner.class)
public class GoogleLocationEngineImplTest {
  @Mock
  private FusedLocationProviderClient fusedLocationProviderClientMock;

  private LocationEngine engine;
  private GoogleLocationEngineImpl googleLocationEngineImpl;

  @Before
  public void setUp() {
    googleLocationEngineImpl = new GoogleLocationEngineImpl(fusedLocationProviderClientMock);
    engine = new LocationEngineProxy<>(googleLocationEngineImpl);
  }

  @Test
  public void removeLocationUpdatesForInvalidListener() {
    LocationEngineCallback<LocationEngineResult> callback = mock(LocationEngineCallback.class);
    engine.removeLocationUpdates(callback);
    verify(fusedLocationProviderClientMock, never()).removeLocationUpdates(any(LocationCallback.class));
  }

  @Test
  public void removeLocationUpdatesForPendingIntent() {
    PendingIntent pendingIntent = mock(PendingIntent.class);
    engine.removeLocationUpdates(pendingIntent);
    verify(fusedLocationProviderClientMock, times(1))
      .removeLocationUpdates(any(PendingIntent.class));
  }

  @Test(expected = NullPointerException.class)
  public void getLastLocationNullCallback() {
    engine.getLastLocation(null);
  }

  @Test(expected = NullPointerException.class)
  public void requestLocationUpdatesNullCallback() {
    engine.requestLocationUpdates(null, null, null);
  }

  @After
  public void tearDown() {
    reset(fusedLocationProviderClientMock);
    engine = null;
  }
}
