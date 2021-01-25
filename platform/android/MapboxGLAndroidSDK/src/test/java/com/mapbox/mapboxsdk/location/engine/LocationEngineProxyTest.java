package com.mapbox.mapboxsdk.location.engine;

import android.location.LocationListener;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

@RunWith(MockitoJUnitRunner.class)
public class LocationEngineProxyTest {
  @Mock
  private LocationEngineCallback<LocationEngineResult> callback;

  @Mock
  private LocationEngineImpl<LocationListener> engineImpl;

  private LocationEngineProxy<LocationListener> locationEngineProxy;

  @Before
  public void setUp() {
    locationEngineProxy = new LocationEngineProxy<>(engineImpl);
  }

  @Test
  public void testAddListener() {
    AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport transport =
      new AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport(callback);
    when(engineImpl.createListener(callback)).thenReturn(transport);

    LocationListener locationListener = locationEngineProxy.getListener(callback);
    assertThat(locationListener).isSameAs(transport);
    assertThat(locationEngineProxy.getListenersCount()).isEqualTo(1);
  }

  @Test
  public void testAddListenerTwice() {
    AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport transport =
      new AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport(callback);
    when(engineImpl.createListener(callback)).thenReturn(transport);

    locationEngineProxy.getListener(callback);
    locationEngineProxy.getListener(callback);
    assertThat(locationEngineProxy.getListenersCount()).isEqualTo(1);
  }

  @Test
  public void testAddTwoListeners() {
    AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport transport =
      new AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport(callback);
    when(engineImpl.createListener(callback)).thenReturn(transport);
    locationEngineProxy.getListener(callback);

    LocationEngineCallback<LocationEngineResult> anotherCallback = mock(LocationEngineCallback.class);
    AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport anotherTransport =
      new AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport(anotherCallback);
    when(engineImpl.createListener(anotherCallback)).thenReturn(anotherTransport);
    locationEngineProxy.getListener(anotherCallback);
    assertThat(locationEngineProxy.getListenersCount()).isEqualTo(2);
  }

  @Test
  public void testRemoveListener() {
    AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport transport =
      new AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport(callback);
    when(engineImpl.createListener(callback)).thenReturn(transport);
    locationEngineProxy.getListener(callback);

    locationEngineProxy.removeListener(callback);
    assertThat(locationEngineProxy.getListenersCount()).isEqualTo(0);
  }

  @Test
  public void testCheckRemovedListener() {
    AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport transport =
      new AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport(callback);
    when(engineImpl.createListener(callback)).thenReturn(transport);
    locationEngineProxy.getListener(callback);

    LocationEngineCallback<LocationEngineResult> anotherCallback = mock(LocationEngineCallback.class);
    AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport anotherTransport =
      new AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport(anotherCallback);
    when(engineImpl.createListener(anotherCallback)).thenReturn(anotherTransport);
    locationEngineProxy.getListener(anotherCallback);

    assertThat(locationEngineProxy.removeListener(callback)).isSameAs(transport);
    assertThat(locationEngineProxy.removeListener(anotherCallback)).isSameAs(anotherTransport);
  }

  @Test
  public void testRemoveListenerTwice() {
    AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport transport =
      new AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport(callback);
    when(engineImpl.createListener(callback)).thenReturn(transport);
    locationEngineProxy.getListener(callback);

    assertThat(locationEngineProxy.removeListener(callback)).isSameAs(transport);
    assertThat(locationEngineProxy.removeListener(callback)).isNull();
  }
}