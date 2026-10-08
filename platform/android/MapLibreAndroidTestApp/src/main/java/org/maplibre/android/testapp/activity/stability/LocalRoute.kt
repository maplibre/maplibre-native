package org.maplibre.android.testapp.activity.stability

import android.content.Context
import kotlinx.serialization.json.JsonArray
import kotlinx.serialization.json.JsonObject
import kotlinx.serialization.json.JsonPrimitive
import kotlinx.serialization.json.contentOrNull
import kotlinx.serialization.json.doubleOrNull
import kotlinx.serialization.json.jsonArray
import kotlinx.serialization.json.jsonObject
import kotlinx.serialization.json.jsonPrimitive
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.testapp.utils.GeoParseUtil
import org.maplibre.geojson.utils.PolylineUtils
import kotlin.random.Random

/** A local route file decoded to a [RoutePath]. */
class LocalRoute(
    val fileName: String,
    val path: RoutePath,
    val distance: Double,
    val duration: Double,
    val destination: LatLng,
) {
    companion object {
        fun load(context: Context, random: Random): LocalRoute? {
            val files = context.assets.list("routes")
                ?.filter { it.endsWith(".json", ignoreCase = true) }
                .orEmpty()
            if (files.isEmpty()) {
                return null
            }
            val fileName = files.random(random)
            return parse(context, fileName)
        }

        private fun parse(context: Context, fileName: String): LocalRoute? {
            val json = GeoParseUtil.loadStringFromAssets(context, "routes/$fileName")
            val root = kotlinx.serialization.json.Json.parseToJsonElement(json).jsonObject
            val routeJson = root["routes"]?.jsonArray?.firstOrNull()?.jsonObject ?: return null
            val distance = routeJson["distance"]?.jsonPrimitive?.doubleOrNull ?: return null
            val duration = routeJson["duration"]?.jsonPrimitive?.doubleOrNull ?: return null
            val coordinates = coordinates(routeJson["geometry"]) ?: return null
            if (coordinates.size < 2) {
                return null
            }
            val destinationJson = root["waypoints"]?.jsonArray?.lastOrNull()
                ?.jsonObject?.get("location")?.jsonArray
            val destination = if (destinationJson != null && destinationJson.size >= 2) {
                LatLng(
                    destinationJson[1].jsonPrimitive.doubleOrNull ?: return null,
                    destinationJson[0].jsonPrimitive.doubleOrNull ?: return null,
                )
            } else {
                coordinates.last()
            }
            return LocalRoute(fileName, RoutePath(coordinates), distance, duration, destination)
        }

        private fun coordinates(geometry: kotlinx.serialization.json.JsonElement?): List<LatLng>? {
            val primitive = geometry as? JsonPrimitive
            if (primitive?.isString == true) {
                val encoded = primitive.contentOrNull ?: return null
                return PolylineUtils.decode(encoded, 6).map { LatLng(it.latitude(), it.longitude()) }
            }
            val coordinates = (geometry as? JsonObject)?.get("coordinates") as? JsonArray ?: return null
            return coordinates.map { element ->
                val coordinate = element.jsonArray
                LatLng(
                    coordinate[1].jsonPrimitive.doubleOrNull ?: return null,
                    coordinate[0].jsonPrimitive.doubleOrNull ?: return null,
                )
            }
        }
    }
}
