package com.mapbox.mapboxsdk.location.engine;

import android.location.Location;
import android.location.LocationListener;
import android.os.Looper;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.junit.MockitoJUnitRunner;
import org.mockito.stubbing.Answer;
import org.mockito.stubbing.Stubber;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicReference;

import static java.util.concurrent.TimeUnit.SECONDS;
import static junit.framework.TestCase.assertTrue;
import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.when;

@RunWith(MockitoJUnitRunner.class)
public class LocationEngineTest {
  private static final double LATITUDE = 37.7749;
  private static final double LONGITUDE = 122.4194;
  private static final long INTERVAL = 1000L;

  @Mock
  private LocationEngineImpl<LocationListener> locationEngineImpl;

  private LocationEngine engine;

  @Before
  public void setUp() {
    engine = new LocationEngineProxy<>(locationEngineImpl);
  }

  @Test
  public void getLastLocation() throws InterruptedException {
    final CountDownLatch latch = new CountDownLatch(1);
    final AtomicReference<LocationEngineResult> resultRef = new AtomicReference<>();

    LocationEngineCallback<LocationEngineResult> callback = getCallback(resultRef, latch);
    final Location location = getMockLocation(LATITUDE, LONGITUDE);
    final LocationEngineResult expectedResult = getMockEngineResult(location);

    setupDoAnswer(expectedResult).when(locationEngineImpl).getLastLocation(callback);

    engine.getLastLocation(callback);
    assertTrue(latch.await(5, SECONDS));

    LocationEngineResult result = resultRef.get();
    assertThat(result).isSameAs(expectedResult);
    assertThat(result.getLastLocation()).isEqualTo(expectedResult.getLastLocation());
  }

  @Test(expected = NullPointerException.class)
  public void getLastLocationNullCallback() {
    engine.getLastLocation(null);
  }

  @Test
  public void requestLocationUpdates() throws InterruptedException {
    final CountDownLatch latch = new CountDownLatch(1);
    final AtomicReference<LocationEngineResult> resultRef = new AtomicReference<>();

    LocationEngineCallback<LocationEngineResult> callback = getCallback(resultRef, latch);
    final Location location = getMockLocation(LATITUDE, LONGITUDE);
    final LocationEngineResult expectedResult = getMockEngineResult(location);
    Looper looper = mock(Looper.class);

    AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport transport =
      new AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport(callback);
    when(locationEngineImpl.createListener(callback)).thenReturn(transport);

    engine.requestLocationUpdates(getRequest(INTERVAL), callback, looper);
    transport.onLocationChanged(location);
    assertTrue(latch.await(5, SECONDS));

    LocationEngineResult result = resultRef.get();
    assertThat(result.getLastLocation()).isEqualTo(expectedResult.getLastLocation());
  }

  @Test(expected = NullPointerException.class)
  public void requestLocationUpdatesNullCallback() {
    engine.requestLocationUpdates(null, null, null);
  }

  @After
  public void tearDown() {
    reset(locationEngineImpl);
    engine = null;
  }

  private static Stubber setupDoAnswer(final LocationEngineResult expectedResult) {
    return doAnswer(new Answer() {
      @Override
      public Object answer(InvocationOnMock invocation) {
        LocationEngineCallback<LocationEngineResult> callback = invocation.getArgument(0);
        callback.onSuccess(expectedResult);
        return null;
      }
    });
  }

  private static LocationEngineRequest getRequest(long interval) {
    return new LocationEngineRequest.Builder(interval).build();
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