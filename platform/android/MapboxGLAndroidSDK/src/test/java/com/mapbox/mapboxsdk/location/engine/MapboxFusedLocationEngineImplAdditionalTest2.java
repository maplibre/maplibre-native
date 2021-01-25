package com.mapbox.mapboxsdk.location.engine;

import android.content.Context;
import android.location.Criteria;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.os.Looper;

import androidx.annotation.NonNull;

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
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyFloat;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

@RunWith(MockitoJUnitRunner.class)
public class MapboxFusedLocationEngineImplAdditionalTest2 {
  private static final long INTERVAL = 1000L;
  private static final String PROVIDER = "test_provider";
  private ArrayList<LocationEngineProxy> engines = new ArrayList<>();
  private LocationManager mockLocationManager;
  private Location location = new Location(PROVIDER);

  @Before
  public void setUp() {
    location = mock(Location.class);
    when(location.getLatitude()).thenReturn(1.0);
    when(location.getLongitude()).thenReturn(2.0);
    Context mockContext = mock(Context.class);
    mockLocationManager = mock(LocationManager.class);
    when(mockContext.getSystemService(anyString())).thenReturn(mockLocationManager);
    List<String> providers = new ArrayList<>();
    providers.add(PROVIDER);
    when(mockLocationManager.getAllProviders()).thenReturn(providers);
    when(mockLocationManager.getBestProvider(any(Criteria.class), anyBoolean()))
      .thenReturn(LocationManager.GPS_PROVIDER);
    doAnswer(new Answer<Object>() {
      @Override
      public Object answer(InvocationOnMock invocation) {
        LocationListener listener = (LocationListener) invocation.getArguments()[3];
        listener.onProviderEnabled(PROVIDER);
        listener.onStatusChanged(PROVIDER, LocationProvider.AVAILABLE, null);
        listener.onLocationChanged(location);
        listener.onProviderDisabled(PROVIDER);
        return null;
      }
    }).when(mockLocationManager)
      .requestLocationUpdates(anyString(), anyLong(), anyFloat(), any(LocationListener.class), any(Looper.class));
    engines.add(new LocationEngineProxy<>(new MapboxFusedLocationEngineImpl(mockContext)));
    engines.add(new LocationEngineProxy<>(new AndroidLocationEngineImpl(mockContext)));
  }

  @Test
  public void checkGetLastLocation() throws InterruptedException {
    final CountDownLatch latch = new CountDownLatch(engines.size());

    for (LocationEngineProxy engineProxy : engines) {
      engineProxy.getLastLocation(new LocationEngineCallback<LocationEngineResult>() {
        @Override
        public void onSuccess(LocationEngineResult result) {
        }

        @Override
        public void onFailure(@NonNull Exception exception) {
          assertEquals("Last location unavailable", exception.getLocalizedMessage());
          latch.countDown();
        }
      });
    }
    assertTrue(latch.await(1, TimeUnit.SECONDS));

    when(mockLocationManager.getLastKnownLocation(anyString())).thenReturn(location);
    final CountDownLatch latch1 = new CountDownLatch(engines.size());

    for (LocationEngineProxy engineProxy : engines) {
      engineProxy.getLastLocation(new LocationEngineCallback<LocationEngineResult>() {
        @Override
        public void onSuccess(LocationEngineResult result) {
          List<Location> list = result.getLocations();
          assertEquals(1, list.size());
          assertEquals(1.0, list.get(0).getLatitude(), 0);
          assertEquals(2.0, list.get(0).getLongitude(), 0);
          latch1.countDown();
        }

        @Override
        public void onFailure(@NonNull Exception exception) {

        }
      });

    }
    assertTrue(latch1.await(1, TimeUnit.SECONDS));

  }

  @Test
  public void checkRequestAndRemoveLocationUpdates() throws InterruptedException {
    final CountDownLatch latch = new CountDownLatch(engines.size());

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


    for (LocationEngineProxy engineProxy : engines) {
      engineProxy.requestLocationUpdates(getRequest(INTERVAL, LocationEngineRequest.PRIORITY_HIGH_ACCURACY),
        engineCallback, mock(Looper.class));

      assertTrue(latch.await(1, TimeUnit.SECONDS));

      assertNotNull(engineProxy.removeListener(engineCallback));
    }

  }

  private static LocationEngineRequest getRequest(long interval, int priority) {
    return new LocationEngineRequest.Builder(interval).setPriority(priority).build();
  }
}
