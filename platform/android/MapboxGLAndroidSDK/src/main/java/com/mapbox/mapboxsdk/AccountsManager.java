package com.mapbox.mapboxsdk;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;
import android.text.TextUtils;
import android.text.format.DateUtils;

import com.mapbox.android.accounts.v1.MapboxAccounts;
import com.mapbox.mapboxsdk.constants.MapboxConstants;
import com.mapbox.mapboxsdk.log.Logger;

/**
 * REMOVAL OR MODIFICATION OF THE FOLLOWING CODE VIOLATES THE MAPBOX TERMS
 * OF SERVICE
 * <p>
 * The following code is used to access Mapbox's Mapping APIs.
 * <p>
 * Removal or modification of this code when used with Mapbox's Mapping APIs
 * can result in termination of your agreement and/or your account with
 * Mapbox.
 * <p>
 * Using this code to access Mapbox Mapping APIs from outside the Mapbox Maps
 * SDK also violates the Mapbox Terms of Service. On Android, Mapping APIs
 * should be accessed using the methods documented at
 * https://www.mapbox.com/android.
 * <p>
 * You can access the Mapbox Terms of Service at https://www.mapbox.com/tos/
 */
class AccountsManager {
  private static final String TAG = "Mbgl-AccountsManager";
  private static final String PREFERENCE_USER_ID = "com.mapbox.mapboxsdk.accounts.userid";
  private static final String PREFERENCE_TIMESTAMP = "com.mapbox.mapboxsdk.accounts.timestamp";

  private SharedPreferences sharedPreferences;
  private String userId;
  private long timestamp;

  AccountsManager() {
    initialize();
  }

  @VisibleForTesting
  AccountsManager(SharedPreferences sharedPreferences) {
    this.sharedPreferences = sharedPreferences;
    initialize();
  }

  private void initialize() {
  }

  private ApplicationInfo retrieveApplicationInfo() throws PackageManager.NameNotFoundException {
    return Mapbox.getApplicationContext().getPackageManager().getApplicationInfo(
      Mapbox.getApplicationContext().getPackageName(),
      PackageManager.GET_META_DATA);
  }

  private boolean isExpired() {
    return isExpired(getNow(), timestamp);
  }

  static boolean isExpired(long now, long then) {
    return ((now - then) > DateUtils.HOUR_IN_MILLIS);
  }

  @NonNull
  private SharedPreferences getSharedPreferences() {
    if (sharedPreferences == null) {
      sharedPreferences = Mapbox.getApplicationContext()
        .getSharedPreferences(MapboxConstants.MAPBOX_SHARED_PREFERENCES, Context.MODE_PRIVATE);
    }
    return sharedPreferences;
  }

  static long getNow() {
    return System.currentTimeMillis();
  }

  private synchronized String getUserId() {
    if (!TextUtils.isEmpty(userId)) {
      return userId;
    }

    SharedPreferences sharedPreferences = getSharedPreferences();
    userId = sharedPreferences.getString(PREFERENCE_USER_ID, "");

    if (TextUtils.isEmpty(userId)) {
      userId = generateUserId();
      SharedPreferences.Editor editor = getSharedPreferences().edit();
      editor.putString(PREFERENCE_USER_ID, userId);
      if (!editor.commit()) {
        Logger.e(TAG, "Failed to save user id.");
      }
    }

    return userId;
  }

  @NonNull
  private String generateUserId() {
    return MapboxAccounts.obtainEndUserId();
  }
}
