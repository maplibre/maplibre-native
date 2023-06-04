package org.maplibre.android;

import android.content.Context;

import androidx.annotation.NonNull;

import org.maplibre.android.util.TileServerOptions;

import java.lang.reflect.Field;

public class MapboxInjector {

  private static final String FIELD_INSTANCE = "INSTANCE";

  public static void inject(@NonNull Context context, @NonNull String apiKey,
                            @NonNull TileServerOptions options) {
    Mapbox mapbox = new Mapbox(context, apiKey, options);
    try {
      Field instance = Mapbox.class.getDeclaredField(FIELD_INSTANCE);
      instance.setAccessible(true);
      instance.set(mapbox, mapbox);
    } catch (Exception exception) {
      throw new AssertionError();
    }
  }

  public static void clear() {
    try {
      Field field = Mapbox.class.getDeclaredField(FIELD_INSTANCE);
      field.setAccessible(true);
      field.set(field, null);
    } catch (Exception exception) {
      throw new AssertionError();
    }
  }
}
