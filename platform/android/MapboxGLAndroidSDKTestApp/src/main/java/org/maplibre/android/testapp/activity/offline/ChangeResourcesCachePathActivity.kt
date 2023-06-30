package org.maplibre.android.testapp.activity.offline

import android.content.Context
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.BaseAdapter
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.log.Logger
import org.maplibre.android.offline.OfflineManager
import org.maplibre.android.offline.OfflineRegion
import org.maplibre.android.storage.FileSource
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.databinding.ActivityChangeResourcesCachePathBinding
import java.io.File

class ChangeResourcesCachePathActivity :
    AppCompatActivity(),
    AdapterView.OnItemClickListener,
    FileSource.ResourcesCachePathChangeCallback {

    companion object {
        private const val TAG = "Mbgl-ChangeResourcesCachePathActivity"
    }

    private lateinit var binding: ActivityChangeResourcesCachePathBinding

    private lateinit var adapter: PathAdapter

    private lateinit var offlineManager: OfflineManager

    private val callback = PathChangeCallback(this)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityChangeResourcesCachePathBinding.inflate(layoutInflater)
        setContentView(binding.root)

        adapter = PathAdapter(this, obtainExternalFilesPaths())
        binding.listView.adapter = adapter
        binding.listView.emptyView = binding.empty
        binding.listView.onItemClickListener = this

        offlineManager = OfflineManager.getInstance(this)
    }

    override fun onStart() {
        super.onStart()
        val path: String? = FileSource.getResourcesCachePath(this)
        Toast.makeText(this, "Current path: $path", Toast.LENGTH_LONG).show()
    }

    override fun onDestroy() {
        super.onDestroy()
        callback.onActivityDestroy()
    }

    override fun onItemClick(parent: AdapterView<*>?, view: View?, position: Int, id: Long) {
        binding.listView.onItemClickListener = null
        val path: String = adapter.getItem(position) as String
        FileSource.setResourcesCachePath(path, callback)
    }

    override fun onError(message: String) {
        binding.listView.onItemClickListener = this
        Toast.makeText(this, "Error: $message", Toast.LENGTH_LONG).show()
    }

    override fun onSuccess(path: String) {
        binding.listView.onItemClickListener = this
        Toast.makeText(this, "New path: $path", Toast.LENGTH_LONG).show()

        offlineManager.listOfflineRegions(object : OfflineManager.ListOfflineRegionsCallback {
            override fun onList(offlineRegions: Array<OfflineRegion>?) {
                Logger.i(TAG, "Number of saved offline regions in the new path: ${offlineRegions?.size}")
            }

            override fun onError(error: String) {
                Logger.e(TAG, error)
            }
        })
    }

    private fun Context.obtainExternalFilesPaths(): List<String> {
        val paths = ArrayList<String>()
        paths.add(this.filesDir.absolutePath)
        paths.addAll(obtainExternalFilesPathsKitKat())
        paths.add("${File.separator}invalid${File.separator}cache${File.separator}path")
        return paths
    }

    private fun Context.obtainExternalFilesPathsKitKat(): List<String> {
        val paths = ArrayList<String>()
        val extDirs = this.getExternalFilesDirs(null)
        for (dir in extDirs) {
            if (dir != null) {
                paths.add(dir.absolutePath)
            }
        }
        return paths
    }

    private class PathChangeCallback(private var activity: ChangeResourcesCachePathActivity?) : FileSource.ResourcesCachePathChangeCallback {

        override fun onSuccess(path: String) {
            activity?.onSuccess(path)
        }

        override fun onError(message: String) {
            activity?.onError(message)
        }

        fun onActivityDestroy() {
            activity = null
        }
    }

    class PathAdapter(private val context: Context, private val paths: List<String>) : BaseAdapter() {

        override fun getItem(position: Int): Any {
            return paths[position]
        }

        override fun getItemId(position: Int): Long {
            return position.toLong()
        }

        override fun getCount(): Int {
            return paths.size
        }

        override fun getView(position: Int, convertView: View?, parent: ViewGroup?): View {
            val viewHolder: ViewHolder
            val view: View

            if (convertView == null) {
                viewHolder = ViewHolder()
                view = LayoutInflater.from(context).inflate(android.R.layout.simple_list_item_1, parent, false)
                viewHolder.textView = view.findViewById(android.R.id.text1)
                view?.tag = viewHolder
            } else {
                view = convertView
                viewHolder = view.tag as ViewHolder
            }

            viewHolder.textView?.text = paths[position]

            return view
        }

        class ViewHolder {
            var textView: TextView? = null
        }
    }
}
