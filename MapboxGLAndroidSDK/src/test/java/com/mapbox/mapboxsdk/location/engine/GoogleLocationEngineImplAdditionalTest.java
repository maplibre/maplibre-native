package com.mapbox.mapboxsdk.location.engine;

import android.location.Location;
import android.os.Looper;

import androidx.annotation.NonNull;

import com.google.android.gms.location.FusedLocationProviderClient;
import com.google.android.gms.location.LocationCallback;
import com.google.android.gms.location.LocationRequest;
import com.google.android.gms.location.LocationResult;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.android.gms.tasks.Task;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.junit.MockitoJUnitRunner;
import org.mockito.stubbing.Answer;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

@RunWith(MockitoJUnitRunner.class)
public class GoogleLocationEngineImplAdditionalTest {
  private static final long INTERVAL = 1000L;
  private LocationEngineProxy engine;
  private Location location;
  private List<Location> locationList = new ArrayList<>();
  private FusedLocationProviderClient fusedLocationProviderClient;
  private Task<Void> mockTask;
  private Task<Location> mockLocationTask;

  @Before
  public void setUp() {
    location = mock(Location.class);
    when(location.getLatitude()).thenReturn(1.0);
    when(location.getLongitude()).thenReturn(2.0);
    locationList.clear();
    locationList.add(location);
    fusedLocationProviderClient = mock(FusedLocationProviderClient.class);
    mockTask = mock(Task.class);
    mockLocationTask = mock(Task.class);
    when(fusedLocationProviderClient.getLastLocation()).thenReturn(mockLocationTask);
    when(fusedLocationProviderClient
      .requestLocationUpdates(any(LocationRequest.class), any(LocationCallback.class), any(Looper.class)))
      .thenAnswer(new Answer<Task<Void>>() {
        @Override
        public Task<Void> answer(InvocationOnMock invocation) {
          LocationCallback listener = (LocationCallback) invocation.getArguments()[1];
          listener.onLocationResult(LocationResult.create(locationList));
          return mockTask;
        }
      });

    engine = new LocationEngineProxy<>(new GoogleLocationEngineImpl(fusedLocationProviderClient));
  }

  @Test
  public void checkGetLastLocation() throws InterruptedException {
    final CountDownLatch latch = new CountDownLatch(1);

    when(mockLocationTask.addOnSuccessListener(any(OnSuccessListener.class)))
      .thenAnswer(new Answer<Object>() {
        @Override
        public Object answer(InvocationOnMock invocation) throws Throwable {
          OnSuccessListener listener = (OnSuccessListener) invocation.getArguments()[0];
          listener.onSuccess(location);
          return mock(Task.class);
        }
      });
    engine.getLastLocation(new LocationEngineCallback<LocationEngineResult>() {
      @Override
      public void onSuccess(LocationEngineResult result) {
        List<Location> list = result.getLocations();
        assertEquals(1, list.size());
        assertEquals(1.0, list.get(0).getLatitude(), 0);
        assertEquals(2.0, list.get(0).getLongitude(), 0);
        latch.countDown();
      }

      @Override
      public void onFailure(@NonNull Exception exception) {

      }
    });
    assertTrue(latch.await(1, TimeUnit.SECONDS));
  }

  @Test
  public void checkRequestAndRemoveLocationUpdates() throws InterruptedException {
    final CountDownLatch latch = new CountDownLatch(1);

    LocationEngineCallback<LocationEngineResult> engineCallback = new LocationEngineCallback<LocationEngineResult>() {
      @Override
      public void onSuccess(LocationEngineResult result) {
        List<Location> list = result.getLocations();
        assertEquals(1, list.size());
        assertEquals(1.0, list.get(0).getLatitude(), 0);
        assertEquals(2.0, list.get(0).getLongitude(), 0);
        latch.countDown();
      }

      @Override
      public void onFailure(@NonNull Exception exception) {

      }
    };
    engine.requestLocationUpdates(getRequest(INTERVAL, LocationEngineRequest.PRIORITY_HIGH_ACCURACY),
      engineCallback, mock(Looper.class));
    assertTrue(latch.await(1, TimeUnit.SECONDS));

    assertNotNull(engine.removeListener(engineCallback));
  }

  private static LocationEngineRequest getRequest(long interval, int priority) {
    return new LocationEngineRequest.Builder(interval).setPriority(priority).build();
  }
}
