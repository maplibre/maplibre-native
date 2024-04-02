package org.maplibre.android.testapp.adapter

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.model.activity.Feature
import org.maplibre.android.testapp.utils.FontCache

/**
 * Adapter used for FeatureOverviewActivity.
 *
 *
 * Adapts a Feature to a visual representation to be shown in a RecyclerView.
 *
 */
class FeatureAdapter(private val features: List<Feature>) :
    RecyclerView.Adapter<FeatureAdapter.ViewHolder>() {
    class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        var labelView: TextView
        var descriptionView: TextView

        init {
            val typeface = FontCache["Roboto-Regular.ttf", view.context]
            labelView = view.findViewById<View>(R.id.nameView) as TextView
            labelView.typeface = typeface
            descriptionView = view.findViewById<View>(R.id.descriptionView) as TextView
            descriptionView.typeface = typeface
        }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view =
            LayoutInflater.from(parent.context).inflate(R.layout.item_main_feature, parent, false)
        return ViewHolder(view)
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        holder.labelView.text = features[position].getLabel()
        holder.descriptionView.text = features[position].getDescription()
    }

    override fun getItemCount(): Int {
        return features.size
    }
}
