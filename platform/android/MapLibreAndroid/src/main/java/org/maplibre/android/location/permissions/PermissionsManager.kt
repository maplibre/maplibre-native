package org.maplibre.android.location.permissions;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Build;
import android.util.Log;

import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Helps request permissions at runtime.
 */
public class PermissionsManager {

  private static final String LOG_TAG = "PermissionsManager";
  private static final String COARSE_LOCATION_PERMISSION = Manifest.permission.ACCESS_COARSE_LOCATION;
  private static final String FINE_LOCATION_PERMISSION = Manifest.permission.ACCESS_FINE_LOCATION;
  private static final String BACKGROUND_LOCATION_PERMISSION = "android.permission.ACCESS_BACKGROUND_LOCATION";

  private final int REQUEST_PERMISSIONS_CODE = 0;

  private PermissionsListener listener;

  public PermissionsManager(PermissionsListener listener) {
    this.listener = listener;
  }

  public PermissionsListener getListener() {
    return listener;
  }

  public void setListener(PermissionsListener listener) {
    this.listener = listener;
  }

  private static boolean isPermissionGranted(Context context, String permission) {
    return ContextCompat.checkSelfPermission(context, permission)
      == PackageManager.PERMISSION_GRANTED;
  }

  private static boolean isCoarseLocationPermissionGranted(Context context) {
    return isPermissionGranted(context, COARSE_LOCATION_PERMISSION);
  }

  private static boolean isFineLocationPermissionGranted(Context context) {
    return isPermissionGranted(context, FINE_LOCATION_PERMISSION);
  }

  public static boolean isBackgroundLocationPermissionGranted(Context context) {
    if (Build.VERSION.SDK_INT >= 29) {
      return isPermissionGranted(context, BACKGROUND_LOCATION_PERMISSION);
    }

    return areLocationPermissionsGranted(context);
  }

  public static boolean areLocationPermissionsGranted(Context context) {
    return isCoarseLocationPermissionGranted(context)
      || isFineLocationPermissionGranted(context);
  }

  public static boolean areRuntimePermissionsRequired() {
    return Build.VERSION.SDK_INT >= Build.VERSION_CODES.M;
  }

  public void requestLocationPermissions(Activity activity) {
    try {
      PackageInfo packageInfo = activity.getPackageManager().getPackageInfo(
              activity.getPackageName(), PackageManager.GET_PERMISSIONS);

      String[] requestedPermissions = packageInfo.requestedPermissions;
      if (requestedPermissions != null) {
        List<String> permissionList = Arrays.asList(requestedPermissions);
        boolean fineLocPermission = permissionList.contains(FINE_LOCATION_PERMISSION);
        boolean coarseLocPermission = permissionList.contains(COARSE_LOCATION_PERMISSION);
        boolean backgroundLocPermission = permissionList.contains(BACKGROUND_LOCATION_PERMISSION);

        // Request location permissions
        if (fineLocPermission) {
          requestLocationPermissions(activity, true, backgroundLocPermission);
        } else if (coarseLocPermission) {
          requestLocationPermissions(activity, false, backgroundLocPermission);
        } else {
          Log.w(LOG_TAG, "Location permissions are missing");
        }
      }
    } catch (Exception exception) {
      Log.w(LOG_TAG, exception.getMessage());
    }
  }

  private void requestLocationPermissions(Activity activity, boolean requestFineLocation,
                                          boolean requestBackgroundLocation) {
    List<String> permissions = new ArrayList<>();
    if (requestFineLocation) {
      permissions.add(FINE_LOCATION_PERMISSION);
    } else  {
      permissions.add(COARSE_LOCATION_PERMISSION);
    }

    if (Build.VERSION.SDK_INT >= 29 && requestBackgroundLocation) {
      permissions.add(BACKGROUND_LOCATION_PERMISSION);
    }

    requestPermissions(activity, permissions.toArray(new String[permissions.size()]));
  }

  private void requestPermissions(Activity activity, String[] permissions) {
    ArrayList<String> permissionsToExplain = new ArrayList<>();
    for (String permission : permissions) {
      if (ActivityCompat.shouldShowRequestPermissionRationale(activity, permission)) {
        permissionsToExplain.add(permission);
      }
    }

    if (listener != null && permissionsToExplain.size() > 0) {
      // The developer should show an explanation to the user asynchronously
      listener.onExplanationNeeded(permissionsToExplain);
    }

    ActivityCompat.requestPermissions(activity, permissions, REQUEST_PERMISSIONS_CODE);
  }

  /**
   * You should call this method from your activity onRequestPermissionsResult.
   *
   * @param requestCode  The request code passed in requestPermissions(android.app.Activity, String[], int)
   * @param permissions  The requested permissions. Never null.
   * @param grantResults The grant results for the corresponding permissions which is either
   *                     PERMISSION_GRANTED or PERMISSION_DENIED. Never null.
   */
  public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
    switch (requestCode) {
      case REQUEST_PERMISSIONS_CODE:
        if (listener != null) {
          boolean granted = grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED;
          listener.onPermissionResult(granted);
        }
        break;
      default:
        // Ignored
    }
  }
}
