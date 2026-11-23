package org.maplibre.android.location.permissions

/**
 * Callback used in PermissionsManager
 */
interface PermissionsListener {
    fun onExplanationNeeded(permissionsToExplain: List<String>)

    fun onPermissionResult(granted: Boolean)
}
