package org.maplibre.android;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.AssetManager;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.UiThread;
import timber.log.Timber;

import org.maplibre.android.constants.MapLibreConstants;
import org.maplibre.android.exceptions.MapLibreConfigurationException;
import org.maplibre.android.net.ConnectivityReceiver;
import org.maplibre.android.storage.FileSource;
import org.maplibre.android.util.DefaultStyle;
import org.maplibre.android.util.TileServerOptions;
import org.maplibre.android.utils.ThreadUtils;

/**
 * The entry point to initialize the MapLibre Android SDK.
 * <p>
 * Obtain a reference by calling {@link #getInstance(Context, String, WellKnownTileServer)}.
 * Usually this class is configured in Application#onCreate() and is responsible for the
 * active API key, application context, and connectivity state.
 * </p>
 */
@UiThread
@SuppressLint("StaticFieldLeak")
@Keep
public final class MapLibre {

  private static final String TAG = "Mbgl-MapLibre";
  private static ModuleProvider moduleProvider;
  private static MapLibre INSTANCE;

  private Context context;
  @Nullable
  private String apiKey;
  @Nullable
  private TileServerOptions tileServerOptions;

  /**
   * Get an instance of MapLibre.
   * <p>
   * This class manages the API key, application context, and connectivity state.
   * </p>
   *
   * @param context Android context which holds or is an application context
   * @return the single instance of MapLibre
   */
  @UiThread
  @NonNull
  public static synchronized MapLibre getInstance(@NonNull Context context) {
    ThreadUtils.init(context);
    ThreadUtils.checkThread(TAG);
    if (INSTANCE == null) {
      Context appContext = context.getApplicationContext();
      FileSource.initializeFileDirsPaths(appContext);
      INSTANCE = new MapLibre(appContext, null);
      ConnectivityReceiver.instance(appContext);
    }

    TileServerOptions tileServerOptions = TileServerOptions.get(WellKnownTileServer.MapLibre);
    INSTANCE.tileServerOptions = tileServerOptions;
    INSTANCE.apiKey = null;
    FileSource fileSource = FileSource.getInstance(context);
    fileSource.setTileServerOptions(tileServerOptions);
    fileSource.setApiKey(null);

    return INSTANCE;
  }

  /**
   * Get an instance of MapLibre.
   * <p>
   * This class manages the API key, application context, and connectivity state.
   * </p>
   *
   * @param context Android context which holds or is an application context
   * @param apiKey api key
   * @param tileServer the tile server whose predefined configuration will be used to
   *                   bootstrap the SDK. The predefined configuration includes
   *                   rules for converting resource URLs between normal and canonical forms
   *                   and set of predefined styles available on the server.
   * @return the single instance of MapLibre
   */
  @UiThread
  @NonNull
  public static synchronized MapLibre getInstance(@NonNull Context context, @Nullable String apiKey,
                                                  WellKnownTileServer tileServer) {
    ThreadUtils.init(context);
    ThreadUtils.checkThread(TAG);
    if (INSTANCE == null) {
      Timber.plant();
      Context appContext = context.getApplicationContext();
      FileSource.initializeFileDirsPaths(appContext);
      INSTANCE = new MapLibre(appContext, apiKey);
      ConnectivityReceiver.instance(appContext);
    } else {
      INSTANCE.apiKey = apiKey;
    }

    TileServerOptions tileServerOptions = TileServerOptions.get(tileServer);
    INSTANCE.tileServerOptions = tileServerOptions;
    FileSource fileSource = FileSource.getInstance(context);
    fileSource.setTileServerOptions(tileServerOptions);
    fileSource.setApiKey(apiKey);
    return INSTANCE;
  }

  MapLibre(@NonNull Context context, @Nullable String apiKey) {
    this.context = context;
    this.apiKey = apiKey;
  }

  MapLibre(@NonNull Context context, @Nullable String apiKey, @NonNull TileServerOptions options) {
    this.context = context;
    this.apiKey = apiKey;
    this.tileServerOptions = options;
  }

  /**
   * Get the current active API key for this application.
   *
   * @return API key
   */
  @Nullable
  public static String getApiKey() {
    validateMapLibre();
    return INSTANCE.apiKey;
  }

  /**
   * Set the current active apiKey.
   */
  public static void setApiKey(String apiKey) {
    validateMapLibre();
    throwIfApiKeyInvalid(apiKey);
    INSTANCE.apiKey = apiKey;
    FileSource.getInstance(getApplicationContext()).setApiKey(apiKey);
  }

  /**
   * Get tile server configuration.
   */
  @Nullable
  public static TileServerOptions getTileServerOptions() {
    validateMapLibre();
    return INSTANCE.tileServerOptions;
  }

  /**
   * Get all pre-defined styles
   *
   * @return Array of predefined styles
   */
  @Nullable
  public static DefaultStyle[] getPredefinedStyles() {
    validateMapLibre();
    if (INSTANCE.tileServerOptions != null) {
      return INSTANCE.tileServerOptions.getDefaultStyles();
    }
    return null;
  }

  /**
   * Get predefined style by name
   *
   * @return Predefined style if found
   */
  @Nullable
  public static DefaultStyle getPredefinedStyle(String name) {
    validateMapLibre();
    if (INSTANCE.tileServerOptions != null) {
      DefaultStyle[] styles = INSTANCE.tileServerOptions.getDefaultStyles();
      for (DefaultStyle style : styles) {
        if (style.getName().equalsIgnoreCase(name)) {
          return style;
        }
      }
    }
    return null;
  }

  /**
   * Application context
   *
   * @return the application context
   */
  @NonNull
  public static Context getApplicationContext() {
    validateMapLibre();
    return INSTANCE.context;
  }

  /**
   * Manually sets the connectivity state of the app. This is useful for apps which control their
   * own connectivity state and want to bypass any checks to the ConnectivityManager.
   *
   * @param connected flag to determine the connectivity state, true for connected, false for
   *                  disconnected, and null for ConnectivityManager to determine.
   */
  public static synchronized void setConnected(Boolean connected) {
    validateMapLibre();
    ConnectivityReceiver.instance(INSTANCE.context).setConnected(connected);
  }

  /**
   * Determines whether we have an internet connection available. Please do not rely on this
   * method in your apps. This method is used internally by the SDK.
   *
   * @return true if there is an internet connection, false otherwise
   */
  public static synchronized Boolean isConnected() {
    validateMapLibre();
    return ConnectivityReceiver.instance(INSTANCE.context).isConnected();
  }

  /**
   * Get the module provider
   *
   * @return moduleProvider
   */
  @NonNull
  public static ModuleProvider getModuleProvider() {
    if (moduleProvider == null) {
      moduleProvider = new ModuleProviderImpl();
    }
    return moduleProvider;
  }

  /**
   * Set the module provider. Call this as soon as possible.
   * @param  provider The ModuleProvider instance to set
   */
  public static void setModuleProvider(ModuleProvider provider) {
    moduleProvider = provider;
  }

  /**
   * Runtime validation of MapLibre creation.
   */
  private static void validateMapLibre() {
    if (INSTANCE == null) {
      throw new MapLibreConfigurationException();
    }
  }

  /**
   * Runtime validation of MapLibre access token
   *
   * @param apiKey the access token to validate
   * @return true is valid, false otherwise
   */
  static boolean isApiKeyValid(@Nullable String apiKey) {
    if (apiKey == null) {
      return false;
    }

    apiKey = apiKey.trim().toLowerCase(MapLibreConstants.MAPLIBRE_LOCALE);
    return !apiKey.isEmpty();
  }

  /**
   * Throws exception when access token is invalid
   */
  public static void throwIfApiKeyInvalid(@Nullable String apiKey) {
    if (!isApiKeyValid(apiKey)) {
      throw new MapLibreConfigurationException(
              "A valid API key is required, currently provided key is: " + apiKey);
    }
  }


  /**
   * Internal use. Check if the {@link MapLibre#INSTANCE} is present.
   */
  public static boolean hasInstance() {
    return INSTANCE != null;
  }

  /**
   * Internal use. Returns AssetManager.
   *
   * @return the asset manager
   */
  private static AssetManager getAssetManager() {
    return getApplicationContext().getResources().getAssets();
  }
}
