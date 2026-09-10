package org.maplibre.android.location.permissions

import android.Manifest
import android.app.Activity
import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import android.util.Log
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat

/**
 * Helps request permissions at runtime.
 */
class PermissionsManager(
    var listener: PermissionsListener?,
) {
    private val requestPermissionsCode = 0

    fun requestLocationPermissions(activity: Activity) {
        try {
            val packageInfo =
                activity.packageManager.getPackageInfo(
                    activity.packageName,
                    PackageManager.GET_PERMISSIONS,
                )

            val requestedPermissions = packageInfo.requestedPermissions
            if (requestedPermissions != null) {
                val permissionList = requestedPermissions.asList()
                val fineLocPermission = permissionList.contains(FINE_LOCATION_PERMISSION)
                val coarseLocPermission = permissionList.contains(COARSE_LOCATION_PERMISSION)
                val backgroundLocPermission = permissionList.contains(BACKGROUND_LOCATION_PERMISSION)

                // Request location permissions
                if (fineLocPermission) {
                    requestLocationPermissions(activity, true, backgroundLocPermission)
                } else if (coarseLocPermission) {
                    requestLocationPermissions(activity, false, backgroundLocPermission)
                } else {
                    Log.w(LOG_TAG, "Location permissions are missing")
                }
            }
        } catch (exception: Exception) {
            Log.w(LOG_TAG, exception.message.orEmpty())
        }
    }

    private fun requestLocationPermissions(
        activity: Activity,
        requestFineLocation: Boolean,
        requestBackgroundLocation: Boolean,
    ) {
        val permissions = mutableListOf<String>()
        if (requestFineLocation) {
            permissions.add(FINE_LOCATION_PERMISSION)
        } else {
            permissions.add(COARSE_LOCATION_PERMISSION)
        }

        if (Build.VERSION.SDK_INT >= 29 && requestBackgroundLocation) {
            permissions.add(BACKGROUND_LOCATION_PERMISSION)
        }

        requestPermissions(activity, permissions.toTypedArray())
    }

    private fun requestPermissions(
        activity: Activity,
        permissions: Array<String>,
    ) {
        val permissionsToExplain = mutableListOf<String>()
        for (permission in permissions) {
            if (ActivityCompat.shouldShowRequestPermissionRationale(activity, permission)) {
                permissionsToExplain.add(permission)
            }
        }

        if (permissionsToExplain.isNotEmpty()) {
            // The developer should show an explanation to the user asynchronously
            listener?.onExplanationNeeded(permissionsToExplain)
        }

        ActivityCompat.requestPermissions(activity, permissions, requestPermissionsCode)
    }

    /**
     * You should call this method from your activity onRequestPermissionsResult.
     *
     * @param requestCode  The request code passed in requestPermissions(android.app.Activity, String[], int)
     * @param permissions  The requested permissions. Never null.
     * @param grantResults The grant results for the corresponding permissions which is either
     *                     PERMISSION_GRANTED or PERMISSION_DENIED. Never null.
     */
    fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray,
    ) {
        if (requestCode == requestPermissionsCode) {
            listener?.let {
                val granted = grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED
                it.onPermissionResult(granted)
            }
        }
    }

    companion object {
        private const val LOG_TAG = "PermissionsManager"
        private val COARSE_LOCATION_PERMISSION = Manifest.permission.ACCESS_COARSE_LOCATION
        private val FINE_LOCATION_PERMISSION = Manifest.permission.ACCESS_FINE_LOCATION
        private const val BACKGROUND_LOCATION_PERMISSION = "android.permission.ACCESS_BACKGROUND_LOCATION"

        private fun isPermissionGranted(
            context: Context,
            permission: String,
        ): Boolean = ContextCompat.checkSelfPermission(context, permission) == PackageManager.PERMISSION_GRANTED

        private fun isCoarseLocationPermissionGranted(context: Context): Boolean = isPermissionGranted(context, COARSE_LOCATION_PERMISSION)

        private fun isFineLocationPermissionGranted(context: Context): Boolean = isPermissionGranted(context, FINE_LOCATION_PERMISSION)

        @JvmStatic
        fun isBackgroundLocationPermissionGranted(context: Context): Boolean {
            if (Build.VERSION.SDK_INT >= 29) {
                return isPermissionGranted(context, BACKGROUND_LOCATION_PERMISSION)
            }

            return areLocationPermissionsGranted(context)
        }

        @JvmStatic
        fun areLocationPermissionsGranted(context: Context): Boolean =
            isCoarseLocationPermissionGranted(context) || isFineLocationPermissionGranted(context)

        @JvmStatic
        fun areRuntimePermissionsRequired(): Boolean = Build.VERSION.SDK_INT >= Build.VERSION_CODES.M
    }
}
