// This file is generated. Edit scripts/generate-style-code.mjs, then run `make style-code`.

package org.maplibre.android.testapp.style;

import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.maplibre.android.style.expressions.Expression;
import org.maplibre.android.style.layers.TransitionOptions;
import org.maplibre.android.style.sky.Sky;
import org.maplibre.android.testapp.activity.BaseTest;
import org.maplibre.android.testapp.activity.style.FillExtrusionStyleTestActivity;

import static org.maplibre.android.style.expressions.Expression.interpolate;
import static org.maplibre.android.style.expressions.Expression.linear;
import static org.maplibre.android.style.expressions.Expression.rgba;
import static org.maplibre.android.style.expressions.Expression.stop;
import static org.maplibre.android.style.expressions.Expression.zoom;
import static org.maplibre.android.testapp.action.MapLibreMapAction.invoke;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

@RunWith(AndroidJUnit4ClassRunner.class)
public class SkyTest extends BaseTest {

  @Test
  public void testPropertiesExpressionsTransitionsAndRemoval() {
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      Sky sky = new Sky();
      TransitionOptions transition = new TransitionOptions(300, 100);

      sky.setAtmosphereBlend(0.75f);
      sky.setFogColor("rgba(240,245,255,1)");
      sky.setFogGroundBlend(0.25f);
      sky.setHorizonColor("rgba(150,180,210,1)");
      sky.setHorizonFogBlend(0.5f);
      sky.setSkyColor("rgba(25,158,243,1)");
      sky.setSkyHorizonBlend(0.6f);
      sky.setAtmosphereBlendTransition(transition);
      sky.setFogColorTransition(transition);
      sky.setFogGroundBlendTransition(transition);
      sky.setHorizonColorTransition(transition);
      sky.setHorizonFogBlendTransition(transition);
      sky.setSkyColorTransition(transition);
      sky.setSkyHorizonBlendTransition(transition);
      maplibreMap.getStyle().setSky(sky);

      Sky applied = maplibreMap.getStyle().getSky();
      assertNotNull(applied);
      assertEquals(0.75f, applied.getAtmosphereBlend().getValue(), 0f);
      assertEquals("rgba(240,245,255,1)", applied.getFogColor().getValue());
      assertEquals(0.25f, applied.getFogGroundBlend().getValue(), 0f);
      assertEquals("rgba(150,180,210,1)", applied.getHorizonColor().getValue());
      assertEquals(0.5f, applied.getHorizonFogBlend().getValue(), 0f);
      assertEquals("rgba(25,158,243,1)", applied.getSkyColor().getValue());
      assertEquals(0.6f, applied.getSkyHorizonBlend().getValue(), 0f);
      assertEquals(transition, applied.getAtmosphereBlendTransition());
      assertEquals(transition, applied.getFogColorTransition());
      assertEquals(transition, applied.getFogGroundBlendTransition());
      assertEquals(transition, applied.getHorizonColorTransition());
      assertEquals(transition, applied.getHorizonFogBlendTransition());
      assertEquals(transition, applied.getSkyColorTransition());
      assertEquals(transition, applied.getSkyHorizonBlendTransition());

      Expression atmosphere = interpolate(linear(), zoom(), stop(0, 1f), stop(12, 0f));
      Expression skyColor = interpolate(
        linear(),
        zoom(),
        stop(0, rgba(25, 158, 243, 1f)),
        stop(12, rgba(0, 0, 40, 1f))
      );
      applied.setAtmosphereBlend(atmosphere);
      applied.setSkyColor(skyColor);
      maplibreMap.getStyle().setSky(applied);

      Sky expressions = maplibreMap.getStyle().getSky();
      assertNotNull(expressions);
      assertTrue(expressions.getAtmosphereBlend().isExpression());
      assertEquals(atmosphere, expressions.getAtmosphereBlend().getExpression());
      assertTrue(expressions.getSkyColor().isExpression());
      assertEquals(skyColor, expressions.getSkyColor().getExpression());

      maplibreMap.getStyle().removeSky();
      assertNull(maplibreMap.getStyle().getSky());
    });
  }

  @Override
  protected Class getActivityClass() {
    return FillExtrusionStyleTestActivity.class;
  }
}
