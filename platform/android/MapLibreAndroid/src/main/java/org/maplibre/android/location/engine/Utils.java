package org.maplibre.android.location.engine;

import android.location.Location;

import androidx.annotation.Nullable;

final class Utils {
  private static final int TWO_MINUTES = 1000 * 60 * 2;
  private static final int ACCURACY_THRESHOLD_METERS = 200;

  private Utils() {
    // Prevent instantiation
  }

  /**
   * Ensures that an object reference passed as a parameter to the calling method is not null.
   *
   * @param reference object reference.
   * @param message   exception message to use if check fails.
   * @param <T>       object type.
   * @return validated non-null reference.
   */
  static <T> T checkNotNull(@Nullable T reference, String message) {
    if (reference == null) {
      throw new NullPointerException(message);
    }
    return reference;
  }

  /**
   * Checks if class is on class path
   * @param className of the class to check.
   * @return true if class in on class path, false otherwise.
   */
  static boolean isOnClasspath(String className) {
    boolean isOnClassPath = true;
    try {
      Class.forName(className);
    } catch (ClassNotFoundException exception) {
      isOnClassPath = false;
    }
    return isOnClassPath;
  }

  /**
   * Determines whether one Location reading is better than the current Location fix
   * <p>
   * (c) https://developer.android.com/guide/topics/location/strategies
   *
   * @param location            The new Location that you want to evaluate
   * @param currentBestLocation The current Location fix, to which you want to compare the new one
   */
  static boolean isBetterLocation(Location location, Location currentBestLocation) {
    if (currentBestLocation == null) {
      // A new location is always better than no location
      return true;
    }

    // Check whether the new location fix is newer or older
    long timeDelta = location.getTime() - currentBestLocation.getTime();
    boolean isSignificantlyNewer = timeDelta > TWO_MINUTES;
    boolean isSignificantlyOlder = timeDelta < -TWO_MINUTES;
    boolean isNewer = timeDelta > 0;

    // If it's been more than two minutes since the current location, use the new location
    // because the user has likely moved
    if (isSignificantlyNewer) {
      return true;
      // If the new location is more than two minutes older, it must be worse
    } else if (isSignificantlyOlder) {
      return false;
    }

    // Check whether the new location fix is more or less accurate
    int accuracyDelta = (int) (location.getAccuracy() - currentBestLocation.getAccuracy());
    boolean isLessAccurate = accuracyDelta > 0;
    boolean isMoreAccurate = accuracyDelta < 0;
    boolean isSignificantlyLessAccurate = accuracyDelta > ACCURACY_THRESHOLD_METERS;

    // Check if the old and new location are from the same provider
    boolean isFromSameProvider = isSameProvider(location.getProvider(),
            currentBestLocation.getProvider());

    // Determine location quality using a combination of timeliness and accuracy
    if (isMoreAccurate) {
      return true;
    } else if (isNewer && !isLessAccurate) {
      return true;
    } else if (isNewer && !isSignificantlyLessAccurate && isFromSameProvider) {
      return true;
    }
    return false;
  }

  /**
   * Checks whether two providers are the same
   * <p>
   * (c) https://developer.android.com/guide/topics/location/strategies
   */
  private static boolean isSameProvider(String provider1, String provider2) {
    if (provider1 == null) {
      return provider2 == null;
    }
    return provider1.equals(provider2);
  }
}
