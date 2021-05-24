package com.mapbox.mapboxsdk.maps;

import android.content.Context;

import com.mapbox.mapboxsdk.camera.CameraPosition;
import com.mapbox.mapboxsdk.geometry.LatLng;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.robolectric.RobolectricTestRunner;

import static org.junit.Assert.assertNotNull;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

@RunWith(RobolectricTestRunner.class)
public class AttributionDialogManagerTest {
  @InjectMocks
  Context context = mock(Context.class);

  @InjectMocks
  MapboxMap mapboxMap = mock(MapboxMap.class);

  @InjectMocks
  Style style = mock(Style.class);

  private AttributionDialogManager attributionDialogManager;

  @Before
  public void beforeTest() {
    attributionDialogManager = new AttributionDialogManager(context, mapboxMap);
    cameraPosition = new CameraPosition.Builder(CameraPosition.DEFAULT)
            .tilt(5.0f).zoom(12).bearing(24.0f).target(new LatLng(11.1f, 22.2f)).build();
  }

  @Test
  public void testSanity() {
    assertNotNull("AttributionDialogManager should not be null", attributionDialogManager);
  }
}
