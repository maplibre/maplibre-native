package org.maplibre.android.testapp.activity.render

import android.content.res.AssetManager
import android.graphics.Bitmap
import android.os.Bundle
import android.os.Environment
import android.view.Gravity
import android.view.ViewGroup
import android.widget.FrameLayout
import android.widget.ImageView
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import com.google.gson.Gson
import com.google.gson.JsonObject
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.maplibre.android.snapshotter.MapSnapshot
import org.maplibre.android.snapshotter.MapSnapshotter
import okio.buffer
import okio.source
import timber.log.Timber
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.nio.charset.Charset

/**
 * Activity that generates map snapshots based on the node render test suite.
 */
class RenderTestActivity : AppCompatActivity() {
    private val renderResultMap: MutableMap<RenderTestDefinition, Bitmap> = HashMap()
    private var renderTestDefinitions: List<RenderTestDefinition>? = null
    private var onRenderTestCompletionListener: OnRenderTestCompletionListener? = null
    private var mapSnapshotter: MapSnapshotter? = null
    private var imageView: ImageView? = null
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(ImageView(this@RenderTestActivity).also { imageView = it })
        imageView!!.layoutParams = FrameLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.MATCH_PARENT,
            Gravity.CENTER
        )
    }

    override fun onStop() {
        super.onStop()
        mapSnapshotter?.cancel()
    }

    //
    // Loads the ignore tests from assets folder
    //
    private fun loadRenderIgnoreTask(renderTestActivity: RenderTestActivity) : List<String> = loadIgnoreList(renderTestActivity.assets)

    //
    // Loads the render test definitions from assets folder
    //
    private fun loadRenderDefinitionTask(renderTestActivity: RenderTestActivity) : List<RenderTestDefinition> {
        val definitions: MutableList<RenderTestDefinition> = ArrayList()
        val assetManager = renderTestActivity.assets
        val categories =
            try {
                assetManager.list(RENDER_TEST_BASE_PATH)
            } catch (exception: IOException) {
                Timber.e(exception)
                throw exception
            }

        for (counter in categories!!.indices.reversed()) {
            try {
                val tests = assetManager.list(
                    String.format(
                        "%s/%s",
                        RENDER_TEST_BASE_PATH,
                        categories[counter]
                    )
                )
                for (test in tests!!) {
                    val styleJson = loadStyleJson(assetManager, categories[counter], test)
                    val renderTestStyleDefinition = Gson()
                        .fromJson(styleJson, RenderTestStyleDefinition::class.java)
                    val definition = RenderTestDefinition(
                        categories[counter],
                        test,
                        styleJson,
                        renderTestStyleDefinition
                    )
                    if (!definition.hasOperations()) {
                        if (!EXCLUDED_TESTS.contains(definition.name + "," + definition.category)) {
                            definitions.add(definition)
                        }
                    } else {
                        Timber.e(
                            "could not add test, test requires operations: %s from %s",
                            test,
                            categories[counter]
                        )
                    }
                }
            } catch (exception: Exception) {
                Timber.e(exception)
            }
        }
        return definitions
    }

    private fun startRenderTests(renderTestDefinitions: List<RenderTestDefinition>) {
        this.renderTestDefinitions = renderTestDefinitions
        if (!renderTestDefinitions.isEmpty()) {
            render(renderTestDefinitions[0], renderTestDefinitions.size)
        } else {
            // no definitions, finish test without rendering
            onRenderTestCompletionListener!!.onFinish()
        }
    }

    private fun render(renderTestDefinition: RenderTestDefinition, testSize: Int) {
        Timber.d("Render test %s,%s", renderTestDefinition.name, renderTestDefinition.category)
        mapSnapshotter = RenderTestSnapshotter(this, renderTestDefinition.toOptions())
        mapSnapshotter?.start(
            object : MapSnapshotter.SnapshotReadyCallback {
                override fun onSnapshotReady(snapshot: MapSnapshot) {
                    imageView!!.setImageBitmap(snapshot.bitmap)
                    renderResultMap[renderTestDefinition] = snapshot.bitmap
                    if (renderResultMap.size != testSize) {
                        continueTesting(renderTestDefinition)
                    } else {
                        finishTesting()
                    }
                }
            },
            object : MapSnapshotter.ErrorHandler {
                override fun onError(error: String) {
                    Timber.e(error)
                }
            }
        )
    }

    private fun continueTesting(renderTestDefinition: RenderTestDefinition) {
        val next = renderTestDefinitions!!.indexOf(renderTestDefinition) + 1
        Timber.d("Next test: %s / %s", next, renderTestDefinitions!!.size)
        render(renderTestDefinitions!![next], renderTestDefinitions!!.size)
    }

    private fun finishTesting() {
        lifecycleScope.launch(Dispatchers.IO) {
            saveResultToDiskTask(renderResultMap)
            withContext(Dispatchers.Main) {
                onRenderTestCompletionListener?.onFinish()
            }
        }
    }

    //
    // Save tests results to disk
    //
    private fun saveResultToDiskTask(
        renderResultMap: Map<RenderTestDefinition, Bitmap>
    ) {
        if (isExternalStorageWritable) {
            try {
                val testResultDir = FileUtils.createTestResultRootFolder()
                val basePath = testResultDir.absolutePath
                for (testResult in renderResultMap.entries) {
                    writeResultToDisk(basePath, testResult)
                }
            } catch (exception: Exception) {
                Timber.e(exception)
            }
        }
    }

    private fun writeResultToDisk(
        path: String,
        result: Map.Entry<RenderTestDefinition, Bitmap>
    ) {
        val definition = result.key
        val categoryName = definition.category
        val categoryPath = String.format("%s/%s", path, categoryName)
        FileUtils.createCategoryDirectory(categoryPath)
        val testName = result.key.name
        val testDir = FileUtils.createTestDirectory(categoryPath, testName)
        FileUtils.writeTestResultToDisk(testDir, result.value)
    }

    private val isExternalStorageWritable: Boolean
        get() = Environment.MEDIA_MOUNTED == Environment.getExternalStorageState()

    //
    // Callback configuration to notify test executor of test finishing
    //
    interface OnRenderTestCompletionListener {
        fun onFinish()
    }

    fun setOnRenderTestCompletionListener(listener: OnRenderTestCompletionListener?) {
        onRenderTestCompletionListener = listener
        lifecycleScope.launch(Dispatchers.IO) {
            val ignored = loadRenderIgnoreTask(this@RenderTestActivity)
            withContext(Dispatchers.Main) {
                onLoadIgnoreList(ignored)
            }
        }
    }

    fun onLoadIgnoreList(ignoreList: List<String>) {
        Timber.e("We loaded %s of tests to be ignored", ignoreList.size)
        EXCLUDED_TESTS.addAll(ignoreList)
        lifecycleScope.launch(Dispatchers.IO) {
            val definitions = loadRenderDefinitionTask(this@RenderTestActivity)
            withContext(Dispatchers.Main) {
                startRenderTests(definitions)
            }
         }
    }

    //
    // FileUtils
    //
    private object FileUtils {
        fun createCategoryDirectory(catPath: String) {
            val testResultDir = File(catPath)
            if (testResultDir.exists()) {
                return
            }
            if (!testResultDir.mkdirs()) {
                throw RuntimeException("can't create root test directory")
            }
        }

        fun createTestResultRootFolder(): File {
            val testResultDir = File(
                Environment.getExternalStorageDirectory()
                    .toString() + File.separator + "mapbox" + File.separator + "render"
            )
            if (testResultDir.exists()) {
                // cleanup old files
                deleteRecursive(testResultDir)
            }
            if (!testResultDir.mkdirs()) {
                throw RuntimeException("can't create root test directory")
            }
            return testResultDir
        }

        private fun deleteRecursive(fileOrDirectory: File) {
            if (fileOrDirectory.isDirectory) {
                val files = fileOrDirectory.listFiles()
                if (files != null) {
                    for (file in files) {
                        deleteRecursive(file)
                    }
                }
            }
            if (!fileOrDirectory.delete()) {
                Timber.e("can't delete directory")
            }
        }

        fun createTestDirectory(basePath: String, testName: String?): String {
            val testDir = File("$basePath/$testName")
            if (!testDir.exists()) {
                if (!testDir.mkdir()) {
                    throw RuntimeException("can't create sub directory for $testName")
                }
            }
            return testDir.absolutePath
        }

        fun writeTestResultToDisk(testPath: String, testResult: Bitmap) {
            val filePath = "$testPath/actual.png"
            try {
                FileOutputStream(filePath).use { out ->
                    testResult.compress(
                        Bitmap.CompressFormat.PNG,
                        100,
                        out
                    )
                }
            } catch (exception: IOException) {
                Timber.e(exception)
            }
        }
    }

    companion object {
        private const val TEST_BASE_PATH = "integration"
        private const val RENDER_TEST_BASE_PATH = TEST_BASE_PATH + "/render-tests"

        // We additionally read out excluded tests from `/platform/node/test/ignore.json`
        private val EXCLUDED_TESTS: ArrayList<String?> = object : ArrayList<String?>() {
            init {
                add("overlay,background-opacity")
                add("collision-lines-pitched,debug")
                add("1024-circle,extent")
                add("empty,empty")
                add("rotation-alignment-map,icon-pitch-scaling")
                add("rotation-alignment-viewport,icon-pitch-scaling")
                add("pitch15,line-pitch")
                add("pitch30,line-pitch")
                add("line-placement-true-pitched,text-keep-upright")
                add("180,raster-rotation")
                add("45,raster-rotation")
                add("90,raster-rotation")
                add("overlapping,raster-masking")
                add("missing,raster-loading")
                add("pitchAndBearing,line-pitch")
                add("overdraw,sparse-tileset")
            }
        }

        private fun loadIgnoreList(assets: AssetManager): List<String> {
            val ignores: MutableList<String> = ArrayList()
            try {
                assets.open(String.format("%s/ignores.json", TEST_BASE_PATH)).use { input ->
                    val source = input.source().buffer()
                    val styleJson = source.readByteString().string(Charset.forName("utf-8"))
                    val `object` = Gson().fromJson(styleJson, JsonObject::class.java)
                    for ((key) in `object`.entrySet()) {
                        val parts = key.split("/").toTypedArray()
                        ignores.add(String.format("%s,%s", parts[2], parts[1]))
                    }
                }
            } catch (exception: IOException) {
                Timber.e(exception)
            }
            return ignores
        }

        private fun loadStyleJson(assets: AssetManager, category: String?, test: String): String? {
            var styleJson: String? = null
            try {
                assets.open(
                    String.format(
                        "%s/%s/%s/style.json",
                        RENDER_TEST_BASE_PATH,
                        category,
                        test
                    )
                ).use { input ->
                    val source = input.source().buffer()
                    styleJson = source.readByteString().string(Charset.forName("utf-8"))
                }
            } catch (exception: IOException) {
                Timber.e(exception)
            }
            return styleJson
        }
    }
}
