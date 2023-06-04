package org.maplibre.android.maps;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;

import androidx.test.annotation.UiThreadTest;
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner;

import org.maplibre.android.AppCenter;
import org.maplibre.android.Maplibre;
import org.maplibre.android.exceptions.MaplibreConfigurationException;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4ClassRunner.class)
public class MaplibreTest extends AppCenter {

  private static final String API_KEY = "pk.0000000001";
  private static final String API_KEY_2 = "pk.0000000002";

  @Rule
  public ExpectedException expectedException = ExpectedException.none();

  private String realToken;

  @Before
  public void setup() {
    realToken = Maplibre.getApiKey();
  }

  @Test
  @UiThreadTest
  public void testConnected() {
    assertTrue(Maplibre.isConnected());

    // test manual connectivity
    Maplibre.setConnected(true);
    assertTrue(Maplibre.isConnected());
    Maplibre.setConnected(false);
    assertFalse(Maplibre.isConnected());

    // reset to Android connectivity
    Maplibre.setConnected(null);
    assertTrue(Maplibre.isConnected());
  }

  @Test
  @UiThreadTest
  public void setApiKey() {
    Maplibre.setApiKey(API_KEY);
    assertSame(API_KEY, Maplibre.getApiKey());
    Maplibre.setApiKey(API_KEY_2);
    assertSame(API_KEY_2, Maplibre.getApiKey());
  }

  @Test
  @UiThreadTest
  public void setNullApiKey() {
    expectedException.expect(MaplibreConfigurationException.class);
    expectedException.expectMessage(
      "A valid API key is required, currently provided key is: " + null
    );

    Maplibre.setApiKey(null);
  }

  @After
  public void tearDown() {
    Maplibre.setApiKey(realToken);
  }
}