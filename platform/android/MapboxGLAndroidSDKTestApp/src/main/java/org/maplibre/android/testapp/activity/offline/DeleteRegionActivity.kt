package org.maplibre.android.testapp.activity.offline

import android.content.Context
import android.content.DialogInterface
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.*
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.offline.OfflineManager
import org.maplibre.android.offline.OfflineManager.ListOfflineRegionsCallback
import org.maplibre.android.offline.OfflineRegion
import org.maplibre.android.offline.OfflineRegion.OfflineRegionDeleteCallback
import org.maplibre.android.offline.OfflineRegion.OfflineRegionInvalidateCallback
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.activity.offline.DeleteRegionActivity.OfflineRegionAdapter
import org.maplibre.android.testapp.utils.OfflineUtils
import java.util.*

/**
 * Test activity showing integration of deleting an OfflineRegion.
 */
class DeleteRegionActivity :
    AppCompatActivity(),
    AdapterView.OnItemClickListener,
    AdapterView.OnItemLongClickListener {
    private var adapter: OfflineRegionAdapter? = null
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_offline_region_delete)
        val listView = findViewById<ListView>(R.id.listView)
        listView.adapter = OfflineRegionAdapter(this).also { adapter = it }
        listView.emptyView = findViewById(android.R.id.empty)
        listView.onItemClickListener = this
        listView.onItemLongClickListener = this
    }

    override fun onItemClick(parent: AdapterView<*>?, view: View, position: Int, id: Long) {
        val region = adapter!!.getItem(position)
        val metadata = OfflineUtils.convertRegionName(region.metadata)
        val builder = AlertDialog.Builder(this)
        builder.setTitle("Delete region")
        val input = TextView(this)
        input.text = metadata
        builder.setView(input)
        builder.setPositiveButton("OK") { dialog: DialogInterface?, which: Int -> delete(region) }
        builder.setNegativeButton("Cancel") { dialog: DialogInterface, which: Int -> dialog.cancel() }
        builder.show()
    }

    override fun onItemLongClick(
        parent: AdapterView<*>?,
        view: View,
        position: Int,
        id: Long
    ): Boolean {
        val region = adapter!!.getItem(position)
        region.invalidate(object : OfflineRegionInvalidateCallback {
            override fun onInvalidate() {
                Toast.makeText(
                    this@DeleteRegionActivity,
                    "Invalidate region success",
                    Toast.LENGTH_SHORT
                ).show()
            }

            override fun onError(error: String) {
                Toast.makeText(this@DeleteRegionActivity, "Error:$error", Toast.LENGTH_LONG).show()
            }
        })
        return true
    }

    private fun delete(region: OfflineRegion) {
        region.delete(object : OfflineRegionDeleteCallback {
            override fun onDelete() {
                Toast.makeText(
                    this@DeleteRegionActivity,
                    "Region deleted",
                    Toast.LENGTH_SHORT
                ).show()
                loadOfflineRegions()
            }

            override fun onError(error: String) {
                Toast.makeText(
                    this@DeleteRegionActivity,
                    "Region deletion failed with $error",
                    Toast.LENGTH_LONG
                ).show()
            }
        })
    }

    override fun onStart() {
        super.onStart()
        loadOfflineRegions()
    }

    private fun loadOfflineRegions() {
        OfflineManager.getInstance(this).listOfflineRegions(object : ListOfflineRegionsCallback {
            override fun onList(offlineRegions: Array<OfflineRegion>?) {
                if (offlineRegions != null && offlineRegions.isNotEmpty()) {
                    adapter!!.setOfflineRegions(Arrays.asList(*offlineRegions))
                }
            }

            override fun onError(error: String) {
                Toast.makeText(
                    this@DeleteRegionActivity,
                    "Error loading regions $error",
                    Toast.LENGTH_LONG
                ).show()
            }
        })
    }

    private class OfflineRegionAdapter internal constructor(private val context: Context) :
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

        override fun getView(position: Int, convertView: View?, parent: ViewGroup): View {
            var convertView = convertView
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
