package org.maplibre.android.utils;

public class PlatformUtils {

  public static boolean isTest() {
    return isJUnit() || isRobolectric() || !isAndroid();
  }

  public static boolean isJUnit() {
    try {
      Class.forName("org.junit.Test");
      return true;
    } catch (ClassNotFoundException ignored) {
      return false;
    }
  }

  public static boolean isRobolectric() {
    try {
      Class.forName("org.robolectric.Robolectric");
      return true;
    } catch (ClassNotFoundException ignored) {
      return false;
    }
  }

  public static boolean isAndroid() {
    try {
      Class.forName("android.os.Build");
      return true;
    } catch (ClassNotFoundException ignored) {
      return false;
    }
  }
}
