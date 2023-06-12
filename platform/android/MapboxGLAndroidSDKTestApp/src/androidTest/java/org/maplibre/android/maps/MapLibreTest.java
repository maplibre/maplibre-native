package org.maplibre.android.maps;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertTrue;

import androidx.test.annotation.UiThreadTest;
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner;

import org.maplibre.android.AppCenter;
import org.maplibre.android.MapLibre;
import org.maplibre.android.exceptions.MapLibreConfigurationException;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4ClassRunner.class)
public class MapLibreTest extends AppCenter {

  private static final String API_KEY = "pk.0000000001";
  private static final String API_KEY_2 = "pk.0000000002";

  private String realToken;

  @Before
  public void setup() {
    realToken = MapLibre.getApiKey();
  }

  @Test
  @UiThreadTest
  public void testConnected() {
    assertTrue(MapLibre.isConnected());

    // test manual connectivity
    MapLibre.setConnected(true);
    assertTrue(MapLibre.isConnected());
    MapLibre.setConnected(false);
    assertFalse(MapLibre.isConnected());

    // reset to Android connectivity
    MapLibre.setConnected(null);
    assertTrue(MapLibre.isConnected());
  }

  @Test
  @UiThreadTest
  public void setApiKey() {
    MapLibre.setApiKey(API_KEY);
    assertSame(API_KEY, MapLibre.getApiKey());
    MapLibre.setApiKey(API_KEY_2);
    assertSame(API_KEY_2, MapLibre.getApiKey());
  }

  @Test
  @UiThreadTest
  public void setNullApiKey() {
    assertThrows(MapLibreConfigurationException.class, () -> MapLibre.setApiKey(null));
  }

  @After
  public void tearDown() {
    MapLibre.setApiKey(realToken);
  }
}