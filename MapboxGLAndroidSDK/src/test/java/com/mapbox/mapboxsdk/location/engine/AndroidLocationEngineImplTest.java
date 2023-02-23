package com.mapbox.mapboxsdk.location.engine;

import android.app.PendingIntent;
import android.content.Context;
import android.location.Criteria;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Looper;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicReference;

import static java.util.concurrent.TimeUnit.SECONDS;
import static junit.framework.TestCase.assertTrue;
import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

@RunWith(MockitoJUnitRunner.class)
public class AndroidLocationEngineImplTest {
  private static final double LATITUDE = 37.7749;
  private static final double LONGITUDE = 122.4194;

  @Mock
  private LocationManager locationManagerMock;

  private LocationEngine engine;
  private AndroidLocationEngineImpl androidLocationEngineImpl;

  @Before
  public void setUp() {
    Context context = mock(Context.class);
    when(context.getSystemService(Context.LOCATION_SERVICE)).thenReturn(locationManagerMock);
    androidLocationEngineImpl = new AndroidLocationEngineImpl(context);
    engine = new LocationEngineProxy<>(androidLocationEngineImpl);
  }

  @Test
  public void getLastLocation() throws InterruptedException {
    final CountDownLatch latch = new CountDownLatch(1);
    final AtomicReference<LocationEngineResult> resultRef = new AtomicReference<>();

    LocationEngineCallback<LocationEngineResult> callback = getCallback(resultRef, latch);
    final Location location = getMockLocation(LATITUDE, LONGITUDE);
    final LocationEngineResult expectedResult = getMockEngineResult(location);

    when(locationManagerMock.getLastKnownLocation(anyString())).thenReturn(location);

    engine.getLastLocation(callback);
    assertTrue(latch.await(5, SECONDS));

    LocationEngineResult result = resultRef.get();
    assertThat(result.getLastLocation()).isEqualTo(expectedResult.getLastLocation());
  }

  @Test
  public void createListener() {
    LocationEngineCallback<LocationEngineResult> callback = mock(LocationEngineCallback.class);
    LocationListener locationListener = androidLocationEngineImpl.createListener(callback);
    Location mockLocation = getMockLocation(LATITUDE, LONGITUDE);
    locationListener.onLocationChanged(mockLocation);
    ArgumentCaptor<LocationEngineResult> argument = ArgumentCaptor.forClass(LocationEngineResult.class);
    verify(callback).onSuccess(argument.capture());

    LocationEngineResult result = argument.getValue();
    assertThat(result.getLastLocation()).isSameAs(mockLocation);
  }

  @Test
  public void requestLocationUpdatesWithNoPower() {
    LocationEngineRequest request = new LocationEngineRequest.Builder(10)
      .setPriority(LocationEngineRequest.PRIORITY_NO_POWER).build();
    LocationEngineCallback<LocationEngineResult> callback = mock(LocationEngineCallback.class);
    Looper looper = mock(Looper.class);
    Criteria criteria = mock(Criteria.class);

    engine.requestLocationUpdates(request, callback, looper);
    verify(locationManagerMock, never()).getBestProvider(criteria, true);
  }

  @Test
  public void requestLocationUpdatesBestProviderNull() {
    LocationEngineRequest request = new LocationEngineRequest.Builder(10)
      .setPriority(LocationEngineRequest.PRIORITY_HIGH_ACCURACY).build();
    LocationEngineCallback<LocationEngineResult> callback = mock(LocationEngineCallback.class);
    Looper looper = mock(Looper.class);
    when(locationManagerMock.getBestProvider(any(Criteria.class), anyBoolean())).thenReturn(null);

    engine.requestLocationUpdates(request, callback, looper);
    assertThat(androidLocationEngineImpl.currentProvider).isEqualTo("passive");
  }

  @Test
  public void requestLocationUpdatesWithPendingIntent() {
    LocationEngineRequest request = new LocationEngineRequest.Builder(10)
      .setPriority(LocationEngineRequest.PRIORITY_HIGH_ACCURACY).build();
    PendingIntent pendingIntent = mock(PendingIntent.class);

    when(locationManagerMock.getBestProvider(any(Criteria.class), anyBoolean())).thenReturn(null);

    engine.requestLocationUpdates(request, pendingIntent);
    assertThat(androidLocationEngineImpl.currentProvider).isEqualTo("passive");
  }

  @Test
  public void removeLocationUpdatesForInvalidListener() {
    LocationEngineCallback<LocationEngineResult> callback = mock(LocationEngineCallback.class);
    engine.removeLocationUpdates(callback);
    verify(locationManagerMock, never()).removeUpdates(any(LocationListener.class));
  }

  @Test
  public void removeLocationUpdatesForPendingIntent() {
    PendingIntent pendingIntent = mock(PendingIntent.class);
    engine.removeLocationUpdates(pendingIntent);
    verify(locationManagerMock, times(1)).removeUpdates(any(PendingIntent.class));
  }

  @Test
  public void removeLocationUpdatesForValidListener() {
    LocationEngineCallback<LocationEngineResult> callback = mock(LocationEngineCallback.class);
    LocationEngineRequest request = new LocationEngineRequest.Builder(10)
      .setPriority(LocationEngineRequest.PRIORITY_HIGH_ACCURACY).build();
    engine.requestLocationUpdates(request, callback, mock(Looper.class));
    engine.removeLocationUpdates(callback);
    verify(locationManagerMock, times(1)).removeUpdates(any(LocationListener.class));
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
    reset(locationManagerMock);
    engine = null;
  }

  private static LocationEngineCallback<LocationEngineResult> getCallback(
    final AtomicReference<LocationEngineResult> resultRef,
    final CountDownLatch latch) {
    return new LocationEngineCallback<LocationEngineResult>() {
      @Override
      public void onSuccess(LocationEngineResult result) {
        resultRef.set(result);
        latch.countDown();
      }

      @Override
      public void onFailure(Exception exception) {
        exception.printStackTrace();
      }
    };
  }

  private static LocationEngineResult getMockEngineResult(Location location) {
    return LocationEngineResult.create(location);
  }

  private static Location getMockLocation(double lat, double lon) {
    Location location = mock(Location.class);
    location.setLatitude(lat);
    location.setLongitude(lon);
    return location;
  }
}
