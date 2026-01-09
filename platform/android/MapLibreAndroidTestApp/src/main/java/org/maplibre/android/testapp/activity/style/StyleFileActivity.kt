package org.maplibre.android.testapp.activity.style

import android.content.Context
import android.os.Bundle
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import androidx.lifecycle.lifecycleScope
import com.google.android.material.floatingactionbutton.FloatingActionButton
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Style
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import org.maplibre.android.testapp.utils.ResourceUtils
import timber.log.Timber
import java.io.BufferedWriter
import java.io.File
import java.io.FileWriter

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
        mapView.getMapAsync {
            maplibreMap = it
            maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets")) { style: Style? ->
                val fab = findViewById<FloatingActionButton>(R.id.fab_file)
                fab.setColorFilter(ContextCompat.getColor(this@StyleFileActivity, R.color.primary))
                fab.setOnClickListener { view: View ->
                    lifecycleScope.launch(Dispatchers.IO) {
                        val cacheStylePath = createStyleFileTask(view.context)
                        withContext(Dispatchers.Main) {
                            cacheStylePath?.let {
                                maplibreMap.setStyle(
                                    Style.Builder().fromUri("file://$it")
                                )
                            }
                        }
                    }
                }
                val fabStyleJson = findViewById<FloatingActionButton>(R.id.fab_style_json)
                fabStyleJson.setColorFilter(
                    ContextCompat.getColor(
                        this@StyleFileActivity,
                        R.color.primary
                    )
                )
                fabStyleJson.setOnClickListener { view: View ->
                    lifecycleScope.launch(Dispatchers.IO) {
                        val json = loadStyleFileTask(view.context)
                        withContext(Dispatchers.Main) {
                            Timber.d("Read json, %s", json)
                            maplibreMap.setStyle(Style.Builder().fromJson(json))
                        }
                    }
                }
            }
        }
    }

    /**
     * Task to read a style file from the raw folder
     */
    private fun loadStyleFileTask(context: Context): String =
        try {
            ResourceUtils.readRawResource(context, R.raw.demotiles)
        } catch (exception: Exception) {
            Timber.e(exception, "Can't load local file style")
            ""
        }

    /**
     * Task to write a style file to local disk and load it in the map view
     */
    private fun createStyleFileTask(
        context: Context,
    ): String? {
        return try {
            val cacheStyleFile = File.createTempFile("my-", ".style.json")
            cacheStyleFile.createNewFile()
            Timber.i("Writing style file to: %s", cacheStyleFile.getAbsolutePath())
            writeToFile(
                cacheStyleFile,
                ResourceUtils.readRawResource(context, R.raw.local_style)
            )
            cacheStyleFile.absolutePath
        } catch (exception: Exception) {
            Toast.makeText(
                context,
                "Could not create style file in cache dir",
                Toast.LENGTH_SHORT
            ).show()
            null
        }
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
