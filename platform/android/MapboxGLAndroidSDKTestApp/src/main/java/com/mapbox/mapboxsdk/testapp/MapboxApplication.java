package com.mapbox.mapboxsdk.testapp;

import android.app.Application;
import android.os.StrictMode;
import android.text.TextUtils;

import com.mapbox.mapboxsdk.MapStrictMode;
import com.mapbox.mapboxsdk.Mapbox;
import com.mapbox.mapboxsdk.WellKnownTileServer;
import com.mapbox.mapboxsdk.log.Logger;
import com.mapbox.mapboxsdk.testapp.utils.TileLoadingMeasurementUtils;
import com.mapbox.mapboxsdk.testapp.utils.TimberLogger;
import com.mapbox.mapboxsdk.testapp.utils.ApiKeyUtils;
import com.squareup.leakcanary.LeakCanary;

import timber.log.Timber;

import static timber.log.Timber.DebugTree;

/**
 * Application class of the test application.
 * <p>
 * Initialises components as LeakCanary, Strictmode, Timber and Mapbox
 * </p>
 */
public class MapboxApplication extends Application {

  public static final WellKnownTileServer TILE_SERVER = WellKnownTileServer.MapTiler;

  private static final String DEFAULT_API_KEY = "YOUR_API_KEY_GOES_HERE";
  private static final String API_KEY_NOT_SET_MESSAGE = "In order to run the Test App you need to set a valid "
    + "API key. During development, you can set the MGL_API_KEY environment variable for the SDK to "
    + "automatically include it in the Test App. Otherwise, you can manually include it in the "
    + "res/values/developer-config.xml file in the MapboxGLAndroidSDKTestApp folder.";

  @Override
  public void onCreate() {
    super.onCreate();
    if (!initializeLeakCanary()) {
      return;
    }
    initializeLogger();
    initializeStrictMode();
    initializeMapbox();
  }

  protected boolean initializeLeakCanary() {
    if (LeakCanary.isInAnalyzerProcess(this)) {
      // This process is dedicated to LeakCanary for heap analysis.
      // You should not init your app in this process.
      return false;
    }
    LeakCanary.install(this);
    return true;
  }

  private void initializeLogger() {
    Logger.setLoggerDefinition(new TimberLogger());
    if (BuildConfig.DEBUG) {
      Timber.plant(new DebugTree());
    }
  }

  private void initializeStrictMode() {
    StrictMode.setThreadPolicy(new StrictMode.ThreadPolicy.Builder()
      .detectDiskReads()
      .detectDiskWrites()
      .detectNetwork()
      .penaltyLog()
      .build());
    StrictMode.setVmPolicy(new StrictMode.VmPolicy.Builder()
      .detectLeakedSqlLiteObjects()
      .penaltyLog()
      .penaltyDeath()
      .build());
  }

  private void initializeMapbox() {
    String apiKey = ApiKeyUtils.getApiKey(getApplicationContext());
    validateApiKey(apiKey);

    Mapbox.getInstance(getApplicationContext(), apiKey, TILE_SERVER);

    TileLoadingMeasurementUtils.setUpTileLoadingMeasurement();

    MapStrictMode.setStrictModeEnabled(true);
  }

  private static void validateApiKey(String apiKey) {
    if (TextUtils.isEmpty(apiKey) || apiKey.equals(DEFAULT_API_KEY)) {
      Timber.e(API_KEY_NOT_SET_MESSAGE);
    }
  }
}
