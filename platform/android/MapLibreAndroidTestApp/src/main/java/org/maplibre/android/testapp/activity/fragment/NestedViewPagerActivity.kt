package org.maplibre.android.testapp.activity.fragment

import android.annotation.SuppressLint
import android.content.Context
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.FragmentManager
import androidx.fragment.app.FragmentStatePagerAdapter
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMapOptions
import org.maplibre.android.maps.SupportMapFragment
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.databinding.ActivityRecyclerviewBinding
import org.maplibre.android.testapp.styles.TestStyles

/**
 * TestActivity showcasing how to integrate a MapView in a RecyclerView.
 * <p>
 * It requires calling the correct lifecycle methods when detaching and attaching the View to
 * the RecyclerView with onViewAttachedToWindow and onViewDetachedFromWindow.
 * </p>
 */
@SuppressLint("ClickableViewAccessibility")
class NestedViewPagerActivity : AppCompatActivity() {

    private lateinit var binding: ActivityRecyclerviewBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityRecyclerviewBinding.inflate(layoutInflater)
        setContentView(binding.root)
        binding.recyclerView.layoutManager = androidx.recyclerview.widget.LinearLayoutManager(this)
        binding.recyclerView.adapter = ItemAdapter(this, LayoutInflater.from(this), supportFragmentManager)
    }

    class ItemAdapter(private val context: Context, private val inflater: LayoutInflater, private val fragmentManager: androidx.fragment.app.FragmentManager) : androidx.recyclerview.widget.RecyclerView.Adapter<androidx.recyclerview.widget.RecyclerView.ViewHolder>() {

        private val items = listOf(
            "one", "two", "three", ViewPagerItem(), "four", "five", "six", "seven", "eight", "nine", "ten",
            "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen",
            "nineteen", "twenty", "twenty-one"
        )

        private var mapHolder: ViewPagerHolder? = null

        companion object {
            const val TYPE_VIEWPAGER = 0
            const val TYPE_TEXT = 1
        }

        override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): androidx.recyclerview.widget.RecyclerView.ViewHolder {
            return if (viewType == TYPE_VIEWPAGER) {
                val viewPager = inflater.inflate(R.layout.item_viewpager, parent, false) as androidx.viewpager.widget.ViewPager
                mapHolder = ViewPagerHolder(context, viewPager, fragmentManager)
                return mapHolder as ViewPagerHolder
            } else {
                TextHolder(inflater.inflate(android.R.layout.simple_list_item_1, parent, false) as TextView)
            }
        }

        override fun getItemCount(): Int {
            return items.count()
        }

        override fun onBindViewHolder(holder: androidx.recyclerview.widget.RecyclerView.ViewHolder, position: Int) {
            if (holder.itemViewType == TYPE_TEXT) {
                val textHolder = holder as TextHolder
                textHolder.bind(items[position] as String)
            }
        }

        override fun getItemViewType(position: Int): Int {
            return if (items[position] is ViewPagerItem) {
                TYPE_VIEWPAGER
            } else {
                TYPE_TEXT
            }
        }

        class TextHolder(val textView: TextView) : androidx.recyclerview.widget.RecyclerView.ViewHolder(textView) {
            fun bind(item: String) {
                textView.text = item
            }
        }

        class ViewPagerItem
        class ViewPagerHolder(context: Context, private val viewPager: androidx.viewpager.widget.ViewPager, fragmentManager: androidx.fragment.app.FragmentManager) : androidx.recyclerview.widget.RecyclerView.ViewHolder(viewPager) {
            init {
                viewPager.adapter = MapPagerAdapter(context, fragmentManager)
                viewPager.setOnTouchListener { view, motionEvent ->
                    // Disallow the touch request for recyclerView scroll
                    view.parent.requestDisallowInterceptTouchEvent(true)
                    viewPager.onTouchEvent(motionEvent)
                    false
                }
            }
        }

        class MapPagerAdapter(private val context: Context, fm: FragmentManager) : FragmentStatePagerAdapter(fm, BEHAVIOR_RESUME_ONLY_CURRENT_FRAGMENT) {

            override fun getItem(position: Int): androidx.fragment.app.Fragment {
                val options = MapLibreMapOptions.createFromAttributes(context)
                options.textureMode(true)
                options.doubleTapGesturesEnabled(false)
                options.rotateGesturesEnabled(false)
                options.tiltGesturesEnabled(false)
                options.scrollGesturesEnabled(false)
                options.zoomGesturesEnabled(false)
                when (position) {
                    0 -> {
                        options.camera(CameraPosition.Builder().target(LatLng(34.920526, 102.634774)).zoom(3.0).build())
                        val fragment = SupportMapFragment.newInstance(options)
                        fragment.getMapAsync { maplibreMap -> maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets")) }
                        return fragment
                    }
                    1 -> {
                        return EmptyFragment.newInstance()
                    }
                    2 -> {
                        options.camera(CameraPosition.Builder().target(LatLng(62.326440, 92.764913)).zoom(3.0).build())
                        val fragment = SupportMapFragment.newInstance(options)
                        fragment.getMapAsync { maplibreMap -> maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Pastel")) }
                        return fragment
                    }
                    3 -> {
                        return EmptyFragment.newInstance()
                    }
                    4 -> {
                        options.camera(CameraPosition.Builder().target(LatLng(-25.007786, 133.623852)).zoom(3.0).build())
                        val fragment = SupportMapFragment.newInstance(options)
                        fragment.getMapAsync { maplibreMap -> maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Satellite Hybrid")) }
                        return fragment
                    }
                    5 -> {
                        return EmptyFragment.newInstance()
                    }
                }
                throw IllegalAccessError()
            }

            override fun getCount(): Int {
                return 6
            }
        }

        class EmptyFragment : androidx.fragment.app.Fragment() {
            companion object {
                fun newInstance(): EmptyFragment {
                    return EmptyFragment()
                }
            }

            override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View {
                return TextView(inflater.context)
            }
        }
    }
}
