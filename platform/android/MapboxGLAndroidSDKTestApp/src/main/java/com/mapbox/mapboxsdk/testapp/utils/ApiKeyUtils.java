package com.mapbox.mapboxsdk.testapp.utils;


import android.content.Context;

import androidx.annotation.NonNull;

import com.mapbox.mapboxsdk.Mapbox;

public class ApiKeyUtils {

  /**
   * <p>
   * Returns the ApiKey set in the app resources.
   * </p>
   * It will first search for a api key in the Mapbox object. If not found it
   * will then attempt to load the api key from the
   * {@code res/values/dev.xml} development file.
   *
   * @param context The {@link Context} of the {@link android.app.Activity} or {@link android.app.Fragment}.
   * @return The api key or null if not found.
   */
  public static String getApiKey(@NonNull Context context) {
    try {
      // Read out AndroidManifest
      String apiKey = Mapbox.getApiKey();
      if (apiKey == null || apiKey.isEmpty()) {
        throw new IllegalArgumentException();
      }
      return apiKey;
    } catch (Exception exception) {
      // Use fallback on string resource, used for development
      //TODO:PP
      int apiKeyResId = context.getResources()
        .getIdentifier("api_key", "string", context.getPackageName());
      return apiKeyResId != 0 ? context.getString(apiKeyResId) : null;
    }
  }
}
