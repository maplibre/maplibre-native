package org.maplibre.android.testapp.activity.offline

import android.content.Context
import android.content.DialogInterface
import android.os.Bundle
import android.text.InputType
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.*
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.offline.OfflineManager
import org.maplibre.android.offline.OfflineManager.ListOfflineRegionsCallback
import org.maplibre.android.offline.OfflineRegion
import org.maplibre.android.offline.OfflineRegion.OfflineRegionUpdateMetadataCallback
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.utils.OfflineUtils
import java.util.*

/**
 * Test activity showing integration of updating metadata of an OfflineRegion.
 */
class UpdateMetadataActivity :
    AppCompatActivity(),
    AdapterView.OnItemClickListener,
    AdapterView.OnItemLongClickListener {
    private var adapter: OfflineRegionMetadataAdapter? = null
    private lateinit var mapView: MapView
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_metadata_update)
        val listView = findViewById<ListView>(R.id.listView)
        listView.adapter = OfflineRegionMetadataAdapter(this).also { adapter = it }
        listView.emptyView = findViewById(android.R.id.empty)
        listView.onItemClickListener = this
        listView.onItemLongClickListener = this
    }

    override fun onItemClick(parent: AdapterView<*>?, view: View, position: Int, id: Long) {
        val region = adapter!!.getItem(position)
        val metadata = OfflineUtils.convertRegionName(region.metadata)
        val builder = AlertDialog.Builder(this)
        builder.setTitle("Rename metadata")
        val input = EditText(this)
        input.setText(metadata)
        input.inputType = InputType.TYPE_CLASS_TEXT
        if (metadata != null) {
            input.setSelection(metadata.length)
        }
        builder.setView(input)
        builder.setPositiveButton(
            "OK"
        ) { dialog: DialogInterface?, which: Int ->
            OfflineUtils.convertRegionName(input.text.toString())?.let {
                updateMetadata(
                    region,
                    it
                )
            }
        }
        builder.setNegativeButton("Cancel") { dialog: DialogInterface, which: Int -> dialog.cancel() }
        builder.show()
    }

    override fun onItemLongClick(
        parent: AdapterView<*>?,
        view: View,
        position: Int,
        id: Long
    ): Boolean {
        val container = findViewById<ViewGroup>(R.id.container)
        container.removeAllViews()
        container.addView(MapView(view.context).also { mapView = it })
        mapView.onCreate(null)
        mapView.getMapAsync { map: MapLibreMap ->
            map.setOfflineRegionDefinition(
                adapter!!.getItem(position).definition
            )
        }
        mapView.onStart()
        mapView.onResume()
        return true
    }

    private fun updateMetadata(region: OfflineRegion, metadata: ByteArray) {
        region.updateMetadata(
            metadata,
            object : OfflineRegionUpdateMetadataCallback {
                override fun onUpdate(metadata: ByteArray) {
                    adapter!!.notifyDataSetChanged()
                }

                override fun onError(error: String) {
                    Toast.makeText(
                        this@UpdateMetadataActivity,
                        "Region metadata update failed with $error",
                        Toast.LENGTH_LONG
                    ).show()
                }
            }
        )
    }

    override fun onStart() {
        super.onStart()
        loadOfflineRegions()
    }

    private fun loadOfflineRegions() {
        OfflineManager.getInstance(this).listOfflineRegions(object : ListOfflineRegionsCallback {
            override fun onList(offlineRegions: Array<OfflineRegion>?) {
                if (offlineRegions != null && offlineRegions.size > 0) {
                    adapter!!.setOfflineRegions(Arrays.asList(*offlineRegions))
                }
            }

            override fun onError(error: String) {
                Toast.makeText(
                    this@UpdateMetadataActivity,
                    "Error loading regions $error",
                    Toast.LENGTH_LONG
                ).show()
            }
        })
    }

    override fun onDestroy() {
        super.onDestroy()
        if (mapView != null) {
            mapView.onPause()
            mapView.onStop()
            mapView.onDestroy()
        }
    }

    private class OfflineRegionMetadataAdapter(private val context: Context) :
        BaseAdapter() {
        private var offlineRegions: List<OfflineRegion>
        fun setOfflineRegions(offlineRegions: List<OfflineRegion>) {
            this.offlineRegions = offlineRegions
            notifyDataSetChanged()
        }

        override fun getCount(): Int {
            return offlineRegions.size
        }

        override fun getItem(position: Int): OfflineRegion {
            return offlineRegions[position]
        }

        override fun getItemId(position: Int): Long {
            return position.toLong()
        }

        override fun getView(position: Int, convertViewParam: View?, parent: ViewGroup): View {
            var convertView = convertViewParam
            val holder: ViewHolder
            if (convertView == null) {
                holder = ViewHolder()
                convertView = LayoutInflater.from(context)
                    .inflate(android.R.layout.simple_list_item_1, parent, false)
                holder.text = convertView.findViewById<View>(android.R.id.text1) as TextView
                convertView.tag = holder
            } else {
                holder = convertView.tag as ViewHolder
            }
            holder.text!!.text = OfflineUtils.convertRegionName(getItem(position).metadata)
            return convertView!!
        }

        internal class ViewHolder {
            var text: TextView? = null
        }

        init {
            offlineRegions = ArrayList()
        }
    }
}
