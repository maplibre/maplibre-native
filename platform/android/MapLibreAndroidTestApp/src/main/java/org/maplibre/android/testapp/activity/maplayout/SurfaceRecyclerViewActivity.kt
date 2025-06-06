package org.maplibre.android.testapp.activity.maplayout

import android.annotation.SuppressLint
import android.os.Bundle
import android.view.LayoutInflater
import android.view.ViewGroup
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.maps.MapView
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.databinding.ActivityRecyclerviewBinding
import org.maplibre.android.testapp.styles.TestStyles

/**
 * TestActivity showcasing how to integrate multiple SurfaceView MapViews in a RecyclerView.
 * <p>
 * It requires calling the correct lifecycle methods when detaching and attaching the View to
 * the RecyclerView with onViewAttachedToWindow and onViewDetachedFromWindow.
 * </p>
 */
@SuppressLint("ClickableViewAccessibility")
open class SurfaceRecyclerViewActivity : AppCompatActivity() {

    lateinit var binding: ActivityRecyclerviewBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityRecyclerviewBinding.inflate(layoutInflater)
        setContentView(binding.root)
        binding.recyclerView.layoutManager = androidx.recyclerview.widget.LinearLayoutManager(this)
        binding.recyclerView.adapter = ItemAdapter(this, LayoutInflater.from(this))
    }

    override fun onLowMemory() {
        super.onLowMemory()
        // to release memory, we need to call MapView#onLowMemory
        (binding.recyclerView.adapter as ItemAdapter).onLowMemory()
    }

    override fun onDestroy() {
        super.onDestroy()
        // to perform cleanup, we need to call MapView#onDestroy
        (binding.recyclerView.adapter as ItemAdapter).onDestroy()
    }

    open fun getMapItemLayoutId(): Int {
        return R.layout.item_map_gl
    }

    class ItemAdapter(private val activity: SurfaceRecyclerViewActivity, private val inflater: LayoutInflater) : androidx.recyclerview.widget.RecyclerView.Adapter<androidx.recyclerview.widget.RecyclerView.ViewHolder>() {

        private val items: List<Any> = listOf(
            "one", "two", "three", MapItem(TestStyles.DEMOTILES), "four", "five", MapItem(TestStyles.DEMOTILES), "seven", "eight", "nine", "ten",
            "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen",
            "nineteen", "twenty", "twenty-one",
        )

        private var mapHolders: MutableList<MapHolder> = mutableListOf()

        companion object {
            const val TYPE_MAP = 0
            const val TYPE_TEXT = 1
        }

        override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): androidx.recyclerview.widget.RecyclerView.ViewHolder {
            return if (viewType == TYPE_MAP) {
                val mapView = inflater.inflate(activity.getMapItemLayoutId(), parent, false) as MapView
                val mapHolder = MapHolder(mapView)
                mapHolders.add(mapHolder)
                return mapHolder
            } else {
                TextHolder(inflater.inflate(android.R.layout.simple_list_item_1, parent, false) as TextView)
            }
        }

        override fun onViewAttachedToWindow(holder: androidx.recyclerview.widget.RecyclerView.ViewHolder) {
            super.onViewAttachedToWindow(holder)
            if (holder is MapHolder) {
                val mapView = holder.mapView
                mapView.isEnabled = false
                mapView.onStart()
                mapView.onResume()
            }
        }

        override fun onViewDetachedFromWindow(holder: androidx.recyclerview.widget.RecyclerView.ViewHolder) {
            super.onViewDetachedFromWindow(holder)
            if (holder is MapHolder) {
                val mapView = holder.mapView
                mapView.onPause()
                mapView.onStop()
            }
        }

        override fun getItemCount(): Int {
            return items.count()
        }

        override fun onBindViewHolder(holder: androidx.recyclerview.widget.RecyclerView.ViewHolder, position: Int) {
            if (holder is TextHolder) {
                holder.bind(items[position] as String)
            } else if (holder is MapHolder) {
                holder.bind(items[position] as MapItem)
            }
        }

        override fun getItemViewType(position: Int): Int {
            return if (items[position] is MapItem) {
                TYPE_MAP
            } else {
                TYPE_TEXT
            }
        }

        fun onLowMemory() {
            for (mapHolder in mapHolders) {
                mapHolder.mapView.onLowMemory()
            }
        }

        fun onDestroy() {
            for (mapHolder in mapHolders) {
                mapHolder.mapView.let {
                    it.onPause()
                    it.onStop()
                    it.onDestroy()
                }
            }
        }

        data class MapItem(val style: String)
        class MapHolder(val mapView: MapView) : androidx.recyclerview.widget.RecyclerView.ViewHolder(mapView) {

            init {
                // unfortunately, if there are multiple maps hosted in one activity, state saving is not possible
                mapView.onCreate(null)
                mapView.setOnTouchListener { view, motionEvent ->
                    // Disallow the touch request for recyclerView scroll
                    view.parent.requestDisallowInterceptTouchEvent(true)
                    mapView.onTouchEvent(motionEvent)
                    true
                }
            }

            fun bind(mapItem: MapItem) {
                mapView.getMapAsync { maplibreMap ->
                    maplibreMap.setStyle(mapItem.style) {
                        maplibreMap.snapshot { }
                    }
                }
            }
        }

        class TextHolder(val textView: TextView) : androidx.recyclerview.widget.RecyclerView.ViewHolder(textView) {
            fun bind(item: String) {
                textView.text = item
            }
        }
    }
}
