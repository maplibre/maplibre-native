package org.maplibre.android.testapp.activity.storage

import android.os.Bundle
import android.os.Looper
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.snackbar.Snackbar
import org.maplibre.android.offline.OfflineManager
import org.maplibre.android.testapp.databinding.ActivityCacheManagementBinding

/**
 * Test activity showcasing the cache management APIs
 */
class CacheManagementActivity : AppCompatActivity() {

    private lateinit var binding: ActivityCacheManagementBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityCacheManagementBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val fileSource = OfflineManager.getInstance(this)
        binding.resetDatabaseButton.setOnClickListener {
            fileSource.resetDatabase(object : OfflineManager.FileSourceCallback {
                override fun onSuccess() {
                    showSnackbar("Reset database success")
                }

                override fun onError(message: String) {
                    showSnackbar("Reset database fail: $message")
                }
            })
        }

        binding.invalidateAmbientCacheButton.setOnClickListener {
            fileSource.invalidateAmbientCache(object : OfflineManager.FileSourceCallback {
                override fun onSuccess() {
                    showSnackbar("Invalidate ambient cache success")
                }

                override fun onError(message: String) {
                    showSnackbar("Invalidate ambient cache fail: $message")
                }
            })
        }

        binding.clearAmbientCacheButton.setOnClickListener {
            fileSource.clearAmbientCache(object : OfflineManager.FileSourceCallback {
                override fun onSuccess() {
                    showSnackbar("Clear ambient cache success")
                }

                override fun onError(message: String) {
                    showSnackbar("Clear ambient cache fail: $message")
                }
            })
        }

        binding.setMaximumAmbientCacheSizeButton.setOnClickListener {
            fileSource.setMaximumAmbientCacheSize(
                5000000,
                object : OfflineManager.FileSourceCallback {
                    override fun onSuccess() {
                        showSnackbar("Set maximum ambient cache size success")
                    }

                    override fun onError(message: String) {
                        showSnackbar("Set maximum ambient cache size fail: $message")
                    }
                }
            )
        }
    }

    fun showSnackbar(message: String) {
        // validate that all callbacks occur on main thread
        assert(Looper.myLooper() == Looper.getMainLooper())

        // show snackbar
        Snackbar.make(binding.container, message, Snackbar.LENGTH_SHORT).show()
    }
}
