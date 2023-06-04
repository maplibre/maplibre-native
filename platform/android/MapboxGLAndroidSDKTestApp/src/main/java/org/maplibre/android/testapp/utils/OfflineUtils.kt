package org.maplibre.android.testapp.utils

import com.google.gson.Gson
import com.google.gson.JsonObject
import org.maplibre.android.testapp.activity.offline.OfflineActivity
import timber.log.Timber
import java.lang.Exception

object OfflineUtils {
    fun convertRegionName(metadata: ByteArray): String {
        return try {
            val json = String(metadata, OfflineActivity.JSON_CHARSET)
            val jsonObject = Gson().fromJson(json, JsonObject::class.java)
            val name = jsonObject[OfflineActivity.JSON_FIELD_REGION_NAME].asString
            name ?: ""
        } catch (exception: Exception) {
            ""
        }
    }

    @JvmStatic
    fun convertRegionName(regionName: String?): ByteArray? {
        try {
            val jsonObject = JsonObject()
            jsonObject.addProperty(OfflineActivity.JSON_FIELD_REGION_NAME, regionName)
            return jsonObject.toString().toByteArray(OfflineActivity.JSON_CHARSET)
        } catch (exception: Exception) {
            Timber.e(exception, "Failed to encode metadata: ")
        }
        return null
    }
}
