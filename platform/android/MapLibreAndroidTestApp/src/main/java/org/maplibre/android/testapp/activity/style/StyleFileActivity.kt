package org.maplibre.android.testapp.activity.style

import android.content.Context
import android.os.AsyncTask
import android.os.Bundle
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.google.android.material.floatingactionbutton.FloatingActionButton
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.maps.Style
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import org.maplibre.android.testapp.utils.ResourceUtils
import timber.log.Timber
import java.io.BufferedWriter
import java.io.File
import java.io.FileWriter
import java.lang.ref.WeakReference

/**
 * Test activity showcasing how to use a file:// resource for the style.json and how to use MapLibreMap#setStyleJson.
 */
class StyleFileActivity : AppCompatActivity() {
    private lateinit var maplibreMap: MapLibreMap
    private lateinit var mapView: MapView
    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_style_file)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(
            OnMapReadyCallback { map: MapLibreMap? ->
                if (map != null) {
                    maplibreMap = map
                }
                maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets")) { style: Style? ->
                    val fab = findViewById<FloatingActionButton>(R.id.fab_file)
                    fab.setColorFilter(ContextCompat.getColor(this@StyleFileActivity, R.color.primary))
                    fab.setOnClickListener { view: View ->
                        CreateStyleFileTask(
                            view.context,
                            maplibreMap
                        ).execute()
                    }
                    val fabStyleJson = findViewById<FloatingActionButton>(R.id.fab_style_json)
                    fabStyleJson.setColorFilter(
                        ContextCompat.getColor(
                            this@StyleFileActivity,
                            R.color.primary
                        )
                    )
                    fabStyleJson.setOnClickListener { view: View ->
                        LoadStyleFileTask(
                            view.context,
                            maplibreMap
                        ).execute()
                    }
                }
            }
        )
    }

    /**
     * Task to read a style file from the raw folder
     */
    private class LoadStyleFileTask internal constructor(context: Context, maplibreMap: MapLibreMap?) :
        AsyncTask<Void?, Void?, String>() {
        private val context: WeakReference<Context>
        private val maplibreMap: WeakReference<MapLibreMap?>
        protected override fun doInBackground(vararg p0: Void?): String? {
            var styleJson = ""
            try {
                styleJson = ResourceUtils.readRawResource(context.get(), R.raw.sat_style)
            } catch (exception: Exception) {
                Timber.e(exception, "Can't load local file style")
            }
            return styleJson
        }

        override fun onPostExecute(json: String) {
            super.onPostExecute(json)
            Timber.d("Read json, %s", json)
            val maplibreMap = maplibreMap.get()
            maplibreMap?.setStyle(Style.Builder().fromJson(json))
        }

        init {
            this.context = WeakReference(context)
            this.maplibreMap = WeakReference(maplibreMap)
        }
    }

    /**
     * Task to write a style file to local disk and load it in the map view
     */
    private class CreateStyleFileTask internal constructor(
        context: Context,
        maplibreMap: MapLibreMap?
    ) : AsyncTask<Void?, Int?, Long>() {
        private lateinit var cacheStyleFile: File
        private val context: WeakReference<Context>
        private val maplibreMap: WeakReference<MapLibreMap?>
        protected override fun doInBackground(vararg p0: Void?): Long? {
            try {
                cacheStyleFile = File.createTempFile("my-", ".style.json")
                cacheStyleFile.createNewFile()
                Timber.i("Writing style file to: %s", cacheStyleFile.getAbsolutePath())
                val context = context.get()
                if (context != null) {
                    writeToFile(
                        cacheStyleFile,
                        ResourceUtils.readRawResource(context, R.raw.local_style)
                    )
                }
            } catch (exception: Exception) {
                Toast.makeText(
                    context.get(),
                    "Could not create style file in cache dir",
                    Toast.LENGTH_SHORT
                ).show()
            }
            return 1L
        }

        override fun onPostExecute(result: Long) {
            // Actual file:// usage
            val maplibreMap = maplibreMap.get()
            maplibreMap?.setStyle(
                Style.Builder().fromUri("file://" + cacheStyleFile!!.absolutePath)
            )
        }

        private fun writeToFile(file: File?, contents: String) {
            var writer: BufferedWriter? = null
            try {
                writer = BufferedWriter(FileWriter(file))
                writer.write(contents)
            } finally {
                writer?.close()
            }
        }

        init {
            this.context = WeakReference(context)
            this.maplibreMap = WeakReference(maplibreMap)
        }
    }

    override fun onStart() {
        super.onStart()
        mapView.onStart()
    }

    override fun onResume() {
        super.onResume()
        mapView.onResume()
    }

    override fun onPause() {
        super.onPause()
        mapView.onPause()
    }

    override fun onStop() {
        super.onStop()
        mapView.onStop()
    }

    public override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    public override fun onDestroy() {
        super.onDestroy()
        mapView.onDestroy()
    }
}
