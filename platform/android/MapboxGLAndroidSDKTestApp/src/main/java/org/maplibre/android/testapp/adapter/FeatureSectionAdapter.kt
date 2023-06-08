package org.maplibre.android.testapp.adapter

import android.content.Context
import android.util.SparseArray
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.annotation.IdRes
import androidx.annotation.LayoutRes
import androidx.recyclerview.widget.RecyclerView
import org.maplibre.android.testapp.utils.FontCache
import java.util.*

class FeatureSectionAdapter(
    private val context: Context,
    @param:LayoutRes private val sectionRes: Int,
    @param:IdRes private val textRes: Int,
    private val adapter: FeatureAdapter
) : RecyclerView.Adapter<RecyclerView.ViewHolder>() {

    private val sections = SparseArray<Section>()
    private var valid = true

    private val adapterDataObserver: RecyclerView.AdapterDataObserver =
        object : RecyclerView.AdapterDataObserver() {
            override fun onChanged() {
                valid = adapter.itemCount > 0
                notifyDataSetChanged()
            }

            override fun onItemRangeChanged(positionStart: Int, itemCount: Int) {
                valid = adapter.itemCount > 0
                notifyItemRangeChanged(positionStart, itemCount)
            }

            override fun onItemRangeInserted(positionStart: Int, itemCount: Int) {
                valid = adapter.itemCount > 0
                notifyItemRangeInserted(positionStart, itemCount)
            }

            override fun onItemRangeRemoved(positionStart: Int, itemCount: Int) {
                valid = adapter.itemCount > 0
                notifyItemRangeRemoved(positionStart, itemCount)
            }
        }

    init {
        adapter.registerAdapterDataObserver(adapterDataObserver)
    }

    override fun onCreateViewHolder(parent: ViewGroup, typeView: Int): RecyclerView.ViewHolder {
        return if (typeView == SECTION_TYPE) {
            val view = LayoutInflater.from(context).inflate(sectionRes, parent, false)
            SectionViewHolder(view, textRes)
        } else {
            adapter.onCreateViewHolder(parent, typeView - 1)
        }
    }

    override fun onBindViewHolder(sectionViewHolder: RecyclerView.ViewHolder, position: Int) {
        if (isSectionHeaderPosition(position)) {
            val cleanTitle = sections.get(position).title.toString().replace("_", " ")
            (sectionViewHolder as SectionViewHolder).title.text = cleanTitle
        } else {
            adapter.onBindViewHolder(sectionViewHolder as FeatureAdapter.ViewHolder, getConvertedPosition(position))
        }
    }

    override fun getItemViewType(position: Int): Int {
        return if (isSectionHeaderPosition(position)) {
            SECTION_TYPE
        } else {
            adapter.getItemViewType(getConvertedPosition(position)) + 1
        }
    }

    override fun getItemId(position: Int): Long {
        return if (isSectionHeaderPosition(position)) {
            Integer.MAX_VALUE - sections.indexOfKey(position).toLong()
        } else {
            adapter.getItemId(getConvertedPosition(position))
        }
    }

    override fun getItemCount(): Int {
        return if (valid) adapter.itemCount + sections.size() else 0
    }

    fun setSections(sections: Array<Section>) {
        this.sections.clear()
        Arrays.sort(sections) { section, section1 ->
            if (section.firstPosition == section1.firstPosition) {
                0
            } else if (section.firstPosition < section1.firstPosition) {
                -1
            } else {
                1
            }
        }
        for ((offset, section) in sections.withIndex()) {
            section.sectionedPosition = section.firstPosition + offset
            this.sections.append(section.sectionedPosition, section)
        }
        notifyDataSetChanged()
    }

    fun getConvertedPosition(sectionedPosition: Int): Int {
        if (isSectionHeaderPosition(sectionedPosition)) {
            return RecyclerView.NO_POSITION
        }
        var offset = 0
        for (i in 0 until sections.size()) {
            if (sections.valueAt(i).sectionedPosition > sectionedPosition) {
                break
            }
            --offset
        }
        return sectionedPosition + offset
    }

    fun isSectionHeaderPosition(position: Int): Boolean {
        return sections.get(position) != null
    }

    class SectionViewHolder(view: View, textRes: Int) :
        RecyclerView.ViewHolder(view) {
        val title: TextView = view.findViewById(textRes)

        init {
            title.typeface = FontCache["Roboto-Medium.ttf", view.context]
        }
    }

    class Section(val firstPosition: Int, val title: CharSequence) {
        var sectionedPosition = 0
    }

    companion object {
        private const val SECTION_TYPE = 0
    }
}
