package org.maplibre.android.testapp.utils

import android.content.Context
import android.text.TextUtils
import org.maplibre.geojson.FeatureCollection
import org.maplibre.geojson.Point
import org.maplibre.android.geometry.LatLng
import java.io.BufferedReader
import java.io.IOException
import java.io.InputStreamReader
import java.io.Reader
import java.lang.NullPointerException
import java.lang.StringBuilder
import java.util.ArrayList
import kotlin.Throws

class GeoParseUtil {

    companion object {
        @Throws(IOException::class)
        fun loadStringFromAssets(context: Context, fileName: String?): String {
            if (TextUtils.isEmpty(fileName)) {
                throw NullPointerException("No GeoJSON File Name passed in.")
            }
            val `is` = context.assets.open(fileName!!)
            val rd = BufferedReader(InputStreamReader(`is`, Charsets.UTF_8))
            return readAll(rd)
        }

        fun parseGeoJsonCoordinates(geojsonStr: String?): List<LatLng> {
            val latLngs: MutableList<LatLng> = ArrayList()
            val featureCollection = FeatureCollection.fromJson(geojsonStr!!)
            for (feature in featureCollection.features()!!) {
                if (feature.geometry() is Point) {
                    val point = feature.geometry() as Point?
                    latLngs.add(LatLng(point!!.latitude(), point.longitude()))
                }
            }
            return latLngs
        }

        @Throws(IOException::class)
        private fun readAll(rd: Reader): String {
            val sb = StringBuilder()
            var cp: Int
            while (rd.read().also { cp = it } != -1) {
                sb.append(cp.toChar())
            }
            return sb.toString()
        }
    }
}
