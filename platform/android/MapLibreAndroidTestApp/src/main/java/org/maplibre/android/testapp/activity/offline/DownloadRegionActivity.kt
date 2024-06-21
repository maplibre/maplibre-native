package org.maplibre.android.testapp.activity.offline

import android.annotation.SuppressLint
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.View
import android.widget.ArrayAdapter
import android.widget.SeekBar
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.offline.*
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.databinding.ActivityRegionDownloadBinding
import org.maplibre.android.testapp.styles.TestStyles
import timber.log.Timber
import java.util.*
import java.util.concurrent.TimeUnit

/**
 * Example showcasing how to download an offline region headless, allows to test pausing and resuming the download.
 */
class DownloadRegionActivity : AppCompatActivity(), OfflineRegion.OfflineRegionObserver {

    companion object {
        const val STATUS_UPDATE_TIMEOUT_MS = 10_000L
    }

    private val handler: Handler = Handler(Looper.getMainLooper())
    private lateinit var offlineManager: OfflineManager
    private lateinit var binding: ActivityRegionDownloadBinding
    private var offlineRegion: OfflineRegion? = null
    private var downloading = false
    private var previousCompletedResourceCount: Long = 0
    private var previousUpdateTimestamp: Long = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityRegionDownloadBinding.inflate(layoutInflater)
        setContentView(binding.root)

        offlineManager = OfflineManager.getInstance(this)
        offlineManager.setOfflineMapboxTileCountLimit(Long.MAX_VALUE)
        initUi()

        deleteOldOfflineRegions {
            binding.container.visibility = View.VISIBLE
            binding.fab.visibility = View.VISIBLE
        }
    }

    override fun onDestroy() {
        handler.removeCallbacksAndMessages(null)
        offlineRegion?.setObserver(null)
        super.onDestroy()
    }

    private fun createOfflineRegion() {
        val latitudeNorth = binding.editTextLatNorth.text.toString().toDouble()
        val longitudeEast = binding.editTextLonEast.text.toString().toDouble()
        val latitudeSouth = binding.editTextLatSouth.text.toString().toDouble()
        val longitudeWest = binding.editTextLonWest.text.toString().toDouble()
        val styleUrl = binding.spinnerStyleUrl.selectedItem as String
        val maxZoom = binding.seekbarMaxZoom.progress.toFloat()
        val minZoom = binding.seekbarMinZoom.progress.toFloat()

        if (!validCoordinates(latitudeNorth, longitudeEast, latitudeSouth, longitudeWest)) {
            Toast.makeText(this, "coordinates need to be in valid range", Toast.LENGTH_LONG).show()
            return
        }

        // create offline definition from data
        val definition = OfflineTilePyramidRegionDefinition(
            styleUrl,
            LatLngBounds.Builder()
                .include(LatLng(latitudeNorth, longitudeEast))
                .include(LatLng(latitudeSouth, longitudeWest))
                .build(),
            minZoom.toDouble(),
            maxZoom.toDouble(),
            resources.displayMetrics.density
        )

        logMessage("Creating offline region")
        offlineManager.createOfflineRegion(
            definition,
            byteArrayOf(),
            object : OfflineManager.CreateOfflineRegionCallback {
                override fun onCreate(offlineRegion: OfflineRegion) {
                    logMessage("Region with id ${offlineRegion.id} created")
                    this@DownloadRegionActivity.offlineRegion = offlineRegion
                    startDownload(offlineRegion)
                    binding.fab.visibility = View.VISIBLE
                }

                override fun onError(error: String) {
                    logMessage("Failed to create offline region: $error")
                }
            }
        )
    }

    private fun startDownload(region: OfflineRegion) {
        downloading = true
        binding.fab.setImageResource(R.drawable.ic_pause_black_24dp)
        logMessage("Downloading...")

        region.setObserver(this)
        region.setDownloadState(OfflineRegion.STATE_ACTIVE)

        previousUpdateTimestamp = System.currentTimeMillis()
        handler.postDelayed(
            object : Runnable {
                override fun run() {
                    if (System.currentTimeMillis() > previousUpdateTimestamp + STATUS_UPDATE_TIMEOUT_MS) {
                        logMessage("FAILURE! No progress in ${TimeUnit.MILLISECONDS.toSeconds(STATUS_UPDATE_TIMEOUT_MS)} seconds")
                    } else {
                        handler.postDelayed(this, 1000)
                    }
                }
            },
            1000
        )
    }

    private fun pauseDownload(region: OfflineRegion) {
        downloading = false
        binding.fab.setImageResource(R.drawable.ic_play_arrow_black_24dp)
        handler.removeCallbacksAndMessages(null)
        region.setDownloadState(OfflineRegion.STATE_INACTIVE)
        "Paused".let {
            logMessage(it)
            binding.downloadStatus.text = it
        }
    }

    override fun onStatusChanged(status: OfflineRegionStatus) {
        if (status.isComplete) {
            "Completed".let {
                logMessage("SUCCESS! $it")
                binding.downloadStatus.text = it
            }
            binding.fab.setImageResource(R.drawable.ic_play_arrow_black_24dp)
            handler.removeCallbacksAndMessages(null)
            offlineRegion?.setObserver(null)
            offlineRegion?.setDownloadState(OfflineRegion.STATE_INACTIVE)
        } else {
            val statusText = "Downloaded ${status.completedResourceCount}/${status.requiredResourceCount}"
            statusText.let {
                logMessage(it)
                binding.downloadStatus.text = it
            }

            if (status.completedResourceCount > status.requiredResourceCount &&
                previousCompletedResourceCount <= status.requiredResourceCount
            ) {
                logMessage("FAILURE! Completed > required")
            }
        }

        previousCompletedResourceCount = status.completedResourceCount
        previousUpdateTimestamp = System.currentTimeMillis()
    }

    override fun onError(error: OfflineRegionError) {
        logMessage("Error: $error")
    }

    override fun mapboxTileCountLimitExceeded(limit: Long) {
        logMessage("Error: tile count limit exceeded")
    }

    protected fun logMessage(message: String) {
        Timber.d(message)
        logView.append(message)
        logView.append("\n")
    }

    fun deleteOldOfflineRegions(onCompleted: () -> Unit) {
        offlineManager.listOfflineRegions(object : OfflineManager.ListOfflineRegionsCallback {
            override fun onList(offlineRegions: Array<OfflineRegion>?) {
                val count = offlineRegions?.size ?: 0
                var remainingCount = count
                if (count > 0) {
                    logMessage("Deleting $count old region...")
                    offlineRegions?.forEach {
                        it.delete(object : OfflineRegion.OfflineRegionDeleteCallback {
                            override fun onDelete() {
                                Timber.d("Deleted region with id ${it.id}")
                                onProcessed()
                            }

                            override fun onError(error: String) {
                                Timber.e("Failed to delete region: $error")
                                onProcessed()
                            }

                            private fun onProcessed() {
                                remainingCount--
                                if (remainingCount == 0) {
                                    logMessage("Done deleting")
                                    onCompleted()
                                }
                            }
                        })
                    }
                } else {
                    onCompleted()
                }
            }

            override fun onError(error: String) {
                logMessage("Failed to list offline regions: $error")
                onCompleted()
            }
        })
    }

    // ui

    private val logView: TextView by lazy {
        findViewById<TextView>(R.id.log_text)
    }

    private fun initUi() {
        initEditTexts()
        initSeekbars()
        initSpinner()
        initZoomLevelTextviews()
        initSeekbarListeners()
        initFab()
    }

    @SuppressLint("SetTextI18n")
    private fun initEditTexts() {
        binding.editTextLatNorth.setText("62.0")
        binding.editTextLonEast.setText("24.0")
        binding.editTextLatSouth.setText("60.0")
        binding.editTextLonWest.setText("22.5")
    }

    private fun initSeekbars() {
        val maxZoom = MapLibreConstants.MAXIMUM_ZOOM.toInt()
        binding.seekbarMinZoom.max = maxZoom
        binding.seekbarMinZoom.progress = 1
        binding.seekbarMaxZoom.max = maxZoom
        binding.seekbarMaxZoom.progress = 15
    }

    private fun initSpinner() {
        val styles = ArrayList<String>()
        styles.add(TestStyles.getPredefinedStyleWithFallback("Streets"))
        styles.add(TestStyles.getPredefinedStyleWithFallback("Pastel"))
        styles.add(TestStyles.getPredefinedStyleWithFallback("Bright"))
        styles.add(TestStyles.getPredefinedStyleWithFallback("Outdoor"))
        val spinnerArrayAdapter = ArrayAdapter(this, android.R.layout.simple_spinner_item, styles)
        spinnerArrayAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
        binding.spinnerStyleUrl.adapter = spinnerArrayAdapter
    }

    private fun initZoomLevelTextviews() {
        binding.textViewMaxText.text = String.format("Max zoom: %s", binding.seekbarMaxZoom.progress)
        binding.textViewMinText.text = String.format("Min zoom: %s", binding.seekbarMinZoom.progress)
    }

    private fun initFab() {
        binding.fab.setOnClickListener {
            binding.container.visibility = View.GONE
            if (offlineRegion == null) {
                binding.fab.visibility = View.GONE
                createOfflineRegion()
            } else {
                offlineRegion?.let {
                    if (downloading) {
                        pauseDownload(it)
                    } else {
                        startDownload(it)
                    }
                }
            }
        }
    }

    private fun initSeekbarListeners() {
        binding.seekbarMaxZoom.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                binding.textViewMaxText.text = String.format("Max zoom: %s", progress)
            }

            override fun onStartTrackingTouch(seekBar: SeekBar) {
            }

            override fun onStopTrackingTouch(seekBar: SeekBar) {
            }
        })

        binding.seekbarMinZoom.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                binding.textViewMinText.text = String.format("Min zoom: %s", progress)
            }

            override fun onStartTrackingTouch(seekBar: SeekBar) {
            }

            override fun onStopTrackingTouch(seekBar: SeekBar) {
            }
        })
    }

    private fun validCoordinates(
        latitudeNorth: Double,
        longitudeEast: Double,
        latitudeSouth: Double,
        longitudeWest: Double
    ): Boolean {
        if (latitudeNorth < -90 || latitudeNorth > 90) {
            return false
        } else if (longitudeEast < -180 || longitudeEast > 180) {
            return false
        } else if (latitudeSouth < -90 || latitudeSouth > 90) {
            return false
        } else if (longitudeWest < -180 || longitudeWest > 180) {
            return false
        }
        return true
    }
}
