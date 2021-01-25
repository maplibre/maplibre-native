package com.mapbox.mapboxsdk;

import android.content.Context;

import androidx.annotation.NonNull;

import java.lang.reflect.Field;

import static org.mockito.Mockito.mock;

public class MapboxInjector {

  private static final String FIELD_INSTANCE = "INSTANCE";
  private static final String FIELD_ACCOUNTS = "accounts";

  public static void inject(@NonNull Context context, @NonNull String accessToken) {
    inject(context, accessToken, null);
  }

  public static void inject(@NonNull Context context, @NonNull String accessToken) {
    Mapbox mapbox = new Mapbox(context, accessToken);
    try {
      Field instance = Mapbox.class.getDeclaredField(FIELD_INSTANCE);
      instance.setAccessible(true);
      instance.set(mapbox, mapbox);

      Field accounts = Mapbox.class.getDeclaredField(FIELD_ACCOUNTS);
      accounts.setAccessible(true);

      AccountsManager manager = mock(AccountsManager.class);
      accounts.set(mapbox, manager);
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
