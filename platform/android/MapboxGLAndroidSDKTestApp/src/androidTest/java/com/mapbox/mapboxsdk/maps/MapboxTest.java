package com.mapbox.mapboxsdk.maps;

import androidx.test.annotation.UiThreadTest;
import androidx.test.ext.junit.runners.AndroidJUnit4;

import com.mapbox.mapboxsdk.AppCenter;
import com.mapbox.mapboxsdk.Mapbox;
import com.mapbox.mapboxsdk.exceptions.MapboxConfigurationException;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;

import static junit.framework.Assert.assertFalse;
import static junit.framework.Assert.assertSame;
import static junit.framework.Assert.assertTrue;

@RunWith(AndroidJUnit4.class)
public class MapboxTest extends AppCenter {

  private static final String API_KEY = "pk.0000000001";
  private static final String API_KEY_2 = "pk.0000000002";

  @Rule
  public ExpectedException expectedException = ExpectedException.none();

  private String realToken;

  @Before
  public void setup() {
    realToken = Mapbox.getApiKey();
  }

  @Test
  @UiThreadTest
  public void testConnected() {
    assertTrue(Mapbox.isConnected());

    // test manual connectivity
    Mapbox.setConnected(true);
    assertTrue(Mapbox.isConnected());
    Mapbox.setConnected(false);
    assertFalse(Mapbox.isConnected());

    // reset to Android connectivity
    Mapbox.setConnected(null);
    assertTrue(Mapbox.isConnected());
  }

  @Test
  @UiThreadTest
  public void setApiKey() {
    Mapbox.setApiKey(API_KEY);
    assertSame(API_KEY, Mapbox.getApiKey());
    Mapbox.setApiKey(API_KEY_2);
    assertSame(API_KEY_2, Mapbox.getApiKey());
  }

  @Test
  @UiThreadTest
  public void setNullApiKey() {
    expectedException.expect(MapboxConfigurationException.class);
    expectedException.expectMessage(
      "A valid API key is required, currently provided key is: " + null
    );

    Mapbox.setApiKey(null);
  }

  @After
  public void tearDown() {
    Mapbox.setApiKey(realToken);
  }
}