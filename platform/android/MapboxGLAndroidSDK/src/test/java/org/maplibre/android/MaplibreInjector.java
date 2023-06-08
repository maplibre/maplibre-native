package org.maplibre.android;

import android.content.Context;

import androidx.annotation.NonNull;

import org.maplibre.android.util.TileServerOptions;

import java.lang.reflect.Field;

public class MapLibreInjector {

  private static final String FIELD_INSTANCE = "INSTANCE";

  public static void inject(@NonNull Context context, @NonNull String apiKey,
                            @NonNull TileServerOptions options) {
    MapLibre maplibre = new MapLibre(context, apiKey, options);
    try {
      Field instance = MapLibre.class.getDeclaredField(FIELD_INSTANCE);
      instance.setAccessible(true);
      instance.set(maplibre, maplibre);
    } catch (Exception exception) {
      throw new AssertionError();
    }
  }

  public static void clear() {
    try {
      Field field = MapLibre.class.getDeclaredField(FIELD_INSTANCE);
      field.setAccessible(true);
      field.set(field, null);
    } catch (Exception exception) {
      throw new AssertionError();
    }
  }
}
