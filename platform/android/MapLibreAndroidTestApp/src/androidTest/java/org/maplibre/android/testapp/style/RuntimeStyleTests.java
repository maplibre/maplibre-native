package org.maplibre.android.testapp.style;

import android.graphics.Color;
import android.graphics.PointF;
import android.view.View;

import androidx.test.espresso.UiController;
import androidx.test.espresso.ViewAction;
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner;

import org.maplibre.android.style.layers.CannotAddLayerException;
import org.maplibre.android.style.layers.CircleLayer;
import org.maplibre.android.style.layers.FillLayer;
import org.maplibre.android.style.layers.Layer;
import org.maplibre.android.style.layers.LineLayer;
import org.maplibre.android.style.layers.Property;
import org.maplibre.android.style.layers.PropertyFactory;
import org.maplibre.android.style.sources.CannotAddSourceException;
import org.maplibre.android.style.sources.GeoJsonSource;
import org.maplibre.android.style.sources.RasterSource;
import org.maplibre.android.style.sources.Source;
import org.maplibre.android.style.sources.VectorSource;
import org.maplibre.android.testapp.R;
import org.maplibre.android.testapp.activity.EspressoTest;

import org.hamcrest.Matcher;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.List;

import timber.log.Timber;

import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.matcher.ViewMatchers.isDisplayed;
import static androidx.test.espresso.matcher.ViewMatchers.withId;
import static org.maplibre.android.testapp.action.MapLibreMapAction.invoke;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

/**
 * Basic smoke tests for Layer and Source
 */
@RunWith(AndroidJUnit4ClassRunner.class)
public class RuntimeStyleTests extends EspressoTest {

  @Test
  public void testListLayers() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new BaseViewAction() {

      @Override
      public void perform(UiController uiController, View view) {
        List<Layer> layers = maplibreMap.getStyle().getLayers();
        assertNotNull(layers);
        assertTrue(layers.size() > 0);
        for (Layer layer : layers) {
          assertNotNull(layer);
        }
      }

    });
  }

  @Test
  public void testGetAddRemoveLayer() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new AddRemoveLayerAction());
  }

  @Test
  public void testAddLayerAbove() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new BaseViewAction() {
      @Override
      public void perform(UiController uiController, View view) {
        List<Layer> layers = maplibreMap.getStyle().getLayers();
        Source source = maplibreMap.getStyle().getSources().get(0);

        // Test inserting with invalid above-id
        try {
          maplibreMap.getStyle().addLayerAbove(
            new CircleLayer("invalid-id-layer-test", source.getId()), "no-such-layer-here-man"
          );
          fail("Should have thrown exception");
        } catch (CannotAddLayerException ex) {
          // Yeah
          assertNotNull(ex.getMessage());
        }

        // Insert as last
        CircleLayer last = new CircleLayer("this is the last one", source.getId());
        maplibreMap.getStyle().addLayerAbove(last, layers.get(layers.size() - 1).getId());
        layers = maplibreMap.getStyle().getLayers();
        assertEquals(last.getId(), layers.get(layers.size() - 1).getId());

        // Insert
        CircleLayer second = new CircleLayer("this is the second one", source.getId());
        maplibreMap.getStyle().addLayerAbove(second, layers.get(0).getId());
        layers = maplibreMap.getStyle().getLayers();
        assertEquals(second.getId(), layers.get(1).getId());
      }
    });
  }

  @Test
  public void testRemoveLayerAt() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new BaseViewAction() {

      @Override
      public void perform(UiController uiController, View view) {
        // Remove by index
        Layer firstLayer = maplibreMap.getStyle().getLayers().get(0);
        boolean removed = maplibreMap.getStyle().removeLayerAt(0);
        assertTrue(removed);
        assertNotNull(firstLayer);

        // Test remove by index bounds checks
        Timber.i("Remove layer at index > size");
        assertFalse(maplibreMap.getStyle().removeLayerAt(Integer.MAX_VALUE));
      }
    });
  }

  @Test
  public void testAddLayerAt() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new BaseViewAction() {
      @Override
      public void perform(UiController uiController, View view) {
        List<Layer> layers = maplibreMap.getStyle().getLayers();
        Source source = maplibreMap.getStyle().getSources().get(0);

        // Test inserting out of range
        try {
          maplibreMap.getStyle().addLayerAt(new CircleLayer("invalid-id-layer-test", source.getId()), layers.size());
          fail("Should have thrown exception");
        } catch (CannotAddLayerException ex) {
          // Yeah
          assertNotNull(ex.getMessage());
        }

        // Insert at current last position
        CircleLayer last = new CircleLayer("this is the last one", source.getId());
        maplibreMap.getStyle().addLayerAt(last, layers.size() - 1);
        layers = maplibreMap.getStyle().getLayers();
        assertEquals(last.getId(), layers.get(layers.size() - 2).getId());

        // Insert at start
        CircleLayer second = new CircleLayer("this is the first one", source.getId());
        maplibreMap.getStyle().addLayerAt(second, 0);
        layers = maplibreMap.getStyle().getLayers();
        assertEquals(second.getId(), layers.get(0).getId());
      }
    });
  }


  @Test
  public void testListSources() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new BaseViewAction() {

      @Override
      public void perform(UiController uiController, View view) {
        List<Source> sources = maplibreMap.getStyle().getSources();
        assertNotNull(sources);
        assertTrue(sources.size() > 0);
        for (Source source : sources) {
          assertNotNull(source);
        }
      }

    });
  }

  @Test
  public void testAddRemoveSource() {
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      maplibreMap.getStyle().addSource(new VectorSource("my-source", "maptiler://sources/hillshades"));
      maplibreMap.getStyle().removeSource("my-source");

      // Add initial source
      maplibreMap.getStyle().addSource(new VectorSource("my-source", "maptiler://sources/hillshades"));

      // Remove
      boolean removeOk = maplibreMap.getStyle().removeSource("my-source");
      assertTrue(removeOk);
      assertNull(maplibreMap.getStyle().getLayer("my-source"));

      // Add
      Source source = new VectorSource("my-source", "maptiler://sources/hillshades");
      maplibreMap.getStyle().addSource(source);

      // Remove, preserving the reference
      maplibreMap.getStyle().removeSource(source);

      // Re-add the reference...
      maplibreMap.getStyle().addSource(source);

      // Ensure it's there
      Assert.assertNotNull(maplibreMap.getStyle().getSource(source.getId()));

      // Test adding a duplicate source
      try {
        Source source2 = new VectorSource("my-source", "maptiler://sources/hillshades");
        maplibreMap.getStyle().addSource(source2);
        fail("Should not have been allowed to add a source with a duplicate id");
      } catch (CannotAddSourceException cannotAddSourceException) {
        // OK
      }
    });

  }

  @Test
  public void testVectorSourceUrlGetter() {
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      VectorSource source = new VectorSource("my-source", "maptiler://sources/hillshades");
      maplibreMap.getStyle().addSource(source);
      assertEquals("maptiler://sources/hillshades", source.getUri());
    });
  }

  @Test
  public void testRasterSourceUrlGetter() {
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      RasterSource source = new RasterSource("my-source", "maptiler://sources/hillshades");
      maplibreMap.getStyle().addSource(source);
      assertEquals("maptiler://sources/hillshades", source.getUri());
    });
  }

  @Test
  public void testGeoJsonSourceUrlGetter() {
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      GeoJsonSource source = new GeoJsonSource("my-source");
      maplibreMap.getStyle().addSource(source);
      assertNull(source.getUri());
      source.setUri("http://mapbox.com/my-file.json");
      assertEquals("http://mapbox.com/my-file.json", source.getUri());
    });
  }

  @Test
  public void testRemoveSourceInUse() {
    validateTestSetup();

    onView(withId(R.id.mapView)).perform(new BaseViewAction() {

      @Override
      public void perform(UiController uiController, View view) {
        maplibreMap.getStyle().addSource(new VectorSource("my-source", "maptiler://sources/hillshades"));
        maplibreMap.getStyle().addLayer(new LineLayer("my-layer", "my-source"));
        maplibreMap.getStyle().removeSource("my-source");
        assertNotNull(maplibreMap.getStyle().getSource("my-source"));
      }

    });
  }

  @Test
  public void testRemoveNonExistingSource() {
    invoke(maplibreMap, (uiController, maplibreMap) -> maplibreMap.getStyle().removeSource("source"));
  }

  @Test
  public void testRemoveNonExistingLayer() {
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      assertFalse(maplibreMap.getStyle().removeLayer("layer"));
      assertFalse(maplibreMap.getStyle().removeLayerAt(maplibreMap.getStyle().getLayers().size() + 1));
      assertFalse(maplibreMap.getStyle().removeLayerAt(-1));
    });
  }

  @Test
  public void testRemoveExistingLayer() {
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      Layer firstLayer = maplibreMap.getStyle().getLayers().get(0);
      assertTrue(maplibreMap.getStyle().removeLayer(firstLayer));

      firstLayer = maplibreMap.getStyle().getLayers().get(0);
      assertTrue(maplibreMap.getStyle().removeLayer(firstLayer.getId()));

      assertTrue(maplibreMap.getStyle().removeLayerAt(0));
    });
  }

  /**
   * https://github.com/mapbox/mapbox-gl-native/issues/7973
   */
  @Test
  public void testQueryRenderedFeaturesInputHandling() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new BaseViewAction() {

      @Override
      public void perform(UiController uiController, View view) {
        String[] layerIds = new String[600];
        for (int i = 0; i < layerIds.length; i++) {
          layerIds[i] = "layer-" + i;
        }
        maplibreMap.queryRenderedFeatures(new PointF(100, 100), layerIds);
      }

    });
  }

  private class AddRemoveLayerAction extends BaseViewAction {

    @Override
    public void perform(UiController uiController, View view) {
      // Get initial
      assertNotNull(maplibreMap.getStyle().getLayer("building"));

      // Remove
      boolean removed = maplibreMap.getStyle().removeLayer("building");
      assertTrue(removed);
      assertNull(maplibreMap.getStyle().getLayer("building"));

      // Add
      FillLayer layer = new FillLayer("building", "composite");
      layer.setSourceLayer("building");
      maplibreMap.getStyle().addLayer(layer);
      assertNotNull(maplibreMap.getStyle().getLayer("building"));

      // Assure the reference still works
      layer.setProperties(PropertyFactory.visibility(Property.VISIBLE));

      // Remove, preserving the reference
      maplibreMap.getStyle().removeLayer(layer);

      // Property setters should still work
      layer.setProperties(PropertyFactory.fillColor(Color.RED));

      // Re-add the reference...
      maplibreMap.getStyle().addLayer(layer);

      // Ensure it's there
      Assert.assertNotNull(maplibreMap.getStyle().getLayer(layer.getId()));

      // Test adding a duplicate layer
      try {
        maplibreMap.getStyle().addLayer(new FillLayer("building", "composite"));
        fail("Should not have been allowed to add a layer with a duplicate id");
      } catch (CannotAddLayerException cannotAddLayerException) {
        // OK
      }
    }
  }

  public abstract class BaseViewAction implements ViewAction {

    @Override
    public Matcher<View> getConstraints() {
      return isDisplayed();
    }

    @Override
    public String getDescription() {
      return getClass().getSimpleName();
    }

  }
}
