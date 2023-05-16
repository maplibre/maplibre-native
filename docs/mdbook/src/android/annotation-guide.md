# Annotation Plugin

This guide will show how to use the [Annotations Plugin](https://github.com/maplibre/maplibre-plugins-android/tree/master/plugin-annotation). Using this plugin is preferred way to add annotations over the now deprecated `Marker` class. 

It is possible to add points, circles, polylines and polygons. For each type of  there is a coresponding
annotation manager to handle a group of annotations. You may use a manager
to create and customize the appearance and behavior for the corresponding annotation type.

Here we continue the code from [Quickstart], and use `SymbolManager` for the showcase:

1. In your module Gradle file (usually `<project>/<app-module>/build.gradle`), add the
   Annotation Plugin as a dependency. Replace `<version>` with the latest version
   (e.g.: `1.0.0`). Visit [https:
   //mvnrepository.com/artifact/org.maplibre.gl/android-plugin-annotation-v9][mvn] to
   view the version history.

   Here we also add `okhttp` and `lifecycle` to be able to make HTTP requests.


    ```gradle
    dependencies {
        ...
        implementation 'org.maplibre.gl:android-plugin-annotation-v9:1.0.0'
        implementation 'com.squareup.okhttp3:okhttp:4.10.0'
        implementation 'androidx.lifecycle:lifecycle-runtime-ktx:2.3.1'
        ...
    }
    ```

2. Sync your Android project the with Gradle files.

3. In `MainActivity` we add variables for `MapboxMap` and `SymbolManager`.
   They are used for adding annotations.

    ```kotlin
    private lateinit var mapboxMap: MapboxMap
    private lateinit var symbolManager: SymbolManager
    ```

4. Rewrite `mapview.getMapSync()`. Here we assign values for `mapboxMap` and `SymbolManager`.
   After the style is loaded, add an image into the style for each symbol (id is `marker`),
   fetch data, then add Symbols.

   For more information about `LifecycleScope` usage, please visit the [Android Developer Documentation].

    ```kotlin
    mapView.getMapAsync { map ->
        mapboxMap = map

        map.setStyle("https://demotiles.maplibre.org/style.json") { style ->
            // Add image for symbol
            val markerDrawable = this.resources
                .getDrawable(com.mapbox.mapboxsdk.R.drawable.maplibre_marker_icon_default, null)
            style.addImage("marker", markerDrawable)

            symbolManager = SymbolManager(mapView, map, style)

            lifecycleScope.launch {
                val data = withContext(Dispatchers.IO) { getEarthQuakeDataFromUSGS() } ?: return@launch
                addMarkers(data)
            }
        }
    }
    ```

5. Here we define a time-consuming function to fetch GeoJSON data from a public API:

   ```kotlin
    // Get Earthquake data from usgs.gov: https://earthquake.usgs.gov/fdsnws/event/1/
    private fun getEarthQuakeDataFromUSGS(): FeatureCollection? {
        val url = "https://earthquake.usgs.gov/fdsnws/event/1/query".toHttpUrl().newBuilder()
            .addQueryParameter("format", "geojson")
            .addQueryParameter("starttime", "2022-01-01")
            .addQueryParameter("endtime", "2022-12-31")
            .addQueryParameter("minmagnitude", "6")
            .addQueryParameter("latitude", "24")
            .addQueryParameter("longitude", "121")
            .addQueryParameter("maxradius", "1.5")
            .build()
        val request: Request = Request.Builder().url(url).build()
        val data = OkHttpClient().newCall(request).execute().use { response ->
            response.body?.string() ?: return null
        }

        return FeatureCollection.fromJson(data)
    }
   ```

6. Now it is time to use `SymbolManager` to add annotations:

    ```kotlin
    private fun addMarkers(data: FeatureCollection?) {
        val bounds = mutableListOf<LatLng>()

        // Add symbol for each point feature
        data?.features()?.forEach { feature ->
            val geometry = feature.geometry()?.toJson() ?: return@forEach
            val point = Point.fromJson(geometry) ?: return@forEach
            val latLng = LatLng(point.latitude(), point.longitude())
            bounds.add(latLng)

            val options = SymbolOptions()
                .withLatLng(latLng)
                .withIconImage("marker")
                .withTextOffset(arrayOf(50f, 50f))
                .withData(feature.properties())
            symbolManager.create(options)
        }

        // Move camera to newly added annotations
        mapboxMap.getCameraForLatLngBounds(LatLngBounds.fromLatLngs(bounds))?.let {
            val newCameraPosition = CameraPosition.Builder()
                .target(it.target)
                .zoom(it.zoom - 0.5)
                .build()
            mapboxMap.cameraPosition = newCameraPosition
        }

        // Show alert dialog when click
        symbolManager.addClickListener {
            val title = it.data?.asJsonObject?.get("title")?.asString
            val magnitude = it.data?.asJsonObject?.get("mag")?.asString
            val msg = "$title\nMagnitude: $magnitude"

            AlertDialog.Builder(this@MainActivity).setMessage(msg).create().show()
            return true
        }
    }
    ```

7. Here is the final result after building and running `MainActivity`. The camera should be re-positioned to view the annocations after the data is fetched. Click each annotion, then more information should be displayed in a pop-up.

    <div style="align: center">
        <img src="https://github.com/maplibre/maplibre-native/assets/19887090/ce73a2f3-13a5-46fb-8c7b-70143b019e6c" alt="Screenshot with the map in demotile style">
    </div>

   ```kotlin
    import android.app.AlertDialog
    import com.mapbox.geojson.FeatureCollection
    import android.os.Bundle
    import android.view.LayoutInflater
    import androidx.activity.ComponentActivity
    import androidx.lifecycle.lifecycleScope
    import com.mapbox.geojson.Point
    import com.mapbox.mapboxsdk.Mapbox
    import com.mapbox.mapboxsdk.camera.CameraPosition
    import com.mapbox.mapboxsdk.geometry.LatLng
    import com.mapbox.mapboxsdk.geometry.LatLngBounds
    import com.mapbox.mapboxsdk.maps.MapView
    import com.mapbox.mapboxsdk.maps.MapboxMap
    import com.mapbox.mapboxsdk.plugins.annotation.SymbolManager
    import com.mapbox.mapboxsdk.plugins.annotation.SymbolOptions
    import kotlinx.coroutines.Dispatchers
    import kotlinx.coroutines.launch
    import kotlinx.coroutines.withContext
    import okhttp3.HttpUrl.Companion.toHttpUrl
    import okhttp3.OkHttpClient
    import okhttp3.Request


    class MainActivity : ComponentActivity() {

        // Declare a variable for MapView
        private lateinit var mapView: MapView
        private lateinit var mapboxMap: MapboxMap
        private lateinit var symbolManager: SymbolManager

        override fun onCreate(savedInstanceState: Bundle?) {
            super.onCreate(savedInstanceState)
            // Init MapLibre
            Mapbox.getInstance(this)

            // Init layout view
            val inflater = LayoutInflater.from(this)
            val rootView = inflater.inflate(R.layout.activity_main, null)
            setContentView(rootView)

            // Init the MapView
            mapView = rootView.findViewById(R.id.mapView)
            mapView.getMapAsync { map ->
                mapboxMap = map

                map.setStyle("https://demotiles.maplibre.org/style.json") { style ->
                    // Add image for symbol
                    val markerDrawable = this.resources
                        .getDrawable(com.mapbox.mapboxsdk.R.drawable.maplibre_marker_icon_default, null)
                    style.addImage("marker", markerDrawable)

                    symbolManager = SymbolManager(mapView, map, style)

                    lifecycleScope.launch {
                        val data = withContext(Dispatchers.IO) { getEarthQuakeDataFromUSGS() } ?: return@launch
                        addMarkers(data)
                    }
                }
            }
        }

        private fun addMarkers(data: FeatureCollection?) {
            val bounds = mutableListOf<LatLng>()

            // Add symbol for each point feature
            data?.features()?.forEach { feature ->
                val geometry = feature.geometry()?.toJson() ?: return@forEach
                val point = Point.fromJson(geometry) ?: return@forEach
                val latLng = LatLng(point.latitude(), point.longitude())
                bounds.add(latLng)

                val options = SymbolOptions()
                    .withLatLng(latLng)
                    .withIconImage("marker")
                    .withTextOffset(arrayOf(50f, 50f))
                    .withData(feature.properties())
                symbolManager.create(options)
            }

            // Move camera to newly added annotations
            mapboxMap.getCameraForLatLngBounds(LatLngBounds.fromLatLngs(bounds))?.let {
                val newCameraPosition = CameraPosition.Builder()
                    .target(it.target)
                    .zoom(it.zoom - 0.5)
                    .build()
                mapboxMap.cameraPosition = newCameraPosition
            }

            // Pop up alert dialog when click
            symbolManager.addClickListener {
                val title = it.data?.asJsonObject?.get("title")?.asString
                val magnitude = it.data?.asJsonObject?.get("mag")?.asString
                val msg = "$title\nMagnitude: $magnitude"

                AlertDialog.Builder(this@MainActivity).setMessage(msg).create().show()
                true
            }
        }

        // Get Earthquake data from usgs.gov: https://earthquake.usgs.gov/fdsnws/event/1/
        private fun getEarthQuakeDataFromUSGS(): FeatureCollection? {
            val url = "https://earthquake.usgs.gov/fdsnws/event/1/query".toHttpUrl().newBuilder()
                .addQueryParameter("format", "geojson")
                .addQueryParameter("starttime", "2022-01-01")
                .addQueryParameter("endtime", "2022-12-31")
                .addQueryParameter("minmagnitude", "6")
                .addQueryParameter("latitude", "24")
                .addQueryParameter("longitude", "121")
                .addQueryParameter("maxradius", "1.5")
                .build()
            val request: Request = Request.Builder().url(url).build()
            val data = OkHttpClient().newCall(request).execute().use { response ->
                response.body?.string() ?: return null
            }

            return FeatureCollection.fromJson(data)
        }
    }
   ```

[Quickstart]: ./getting-started-guide.md
[mvn]: https://mvnrepository.com/artifact/org.maplibre.gl/android-plugin-annotation-v9
[Android Developer Documentation]: https://developer.android.com/topic/libraries/architecture/coroutines
