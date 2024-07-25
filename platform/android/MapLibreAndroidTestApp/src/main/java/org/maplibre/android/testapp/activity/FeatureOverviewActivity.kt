package org.maplibre.android.testapp.activity

import android.content.ComponentName
import android.content.Intent
import android.content.pm.PackageInfo
import android.content.pm.PackageManager
import android.content.res.Resources.NotFoundException
import android.os.Bundle
import android.view.View
import androidx.annotation.StringRes
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.RecyclerView.SimpleOnItemTouchListener
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.adapter.FeatureAdapter
import org.maplibre.android.testapp.adapter.FeatureSectionAdapter
import org.maplibre.android.testapp.model.activity.Feature
import org.maplibre.android.testapp.utils.ItemClickSupport
import timber.log.Timber
import java.util.*

/**
 * Activity shown when application is started
 *
 *
 * This activity  will generate data for RecyclerView based on the AndroidManifest entries.
 * It uses tags as category and description to order the different entries.
 *
 */
class FeatureOverviewActivity : AppCompatActivity() {
    private lateinit var recyclerView: RecyclerView
    private var sectionAdapter: FeatureSectionAdapter? = null
    private var features: List<Feature>? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_feature_overview)
        recyclerView = findViewById<View>(R.id.recyclerView) as RecyclerView
        recyclerView.layoutManager = LinearLayoutManager(this)
        recyclerView.addOnItemTouchListener(SimpleOnItemTouchListener())
        recyclerView.setHasFixedSize(true)

        ItemClickSupport.addTo(recyclerView)
            .setOnItemClickListener(object : ItemClickSupport.OnItemClickListener {
                override fun onItemClicked(recyclerView: RecyclerView?, position: Int, view: View?) {
                    if (sectionAdapter!!.isSectionHeaderPosition(position).not()) {
                        val itemPosition = sectionAdapter!!.getConvertedPosition(position)
                        val feature = features!![itemPosition]
                        startFeature(feature)
                    }
                }
            })
        if (savedInstanceState == null) {
            loadFeatures()
        } else {
            features = savedInstanceState.getParcelableArrayList(KEY_STATE_FEATURES)
            onFeaturesLoaded(features)
        }
    }

    private fun loadFeatures() {
        lifecycleScope.launch(Dispatchers.IO) {
            try {
                features = loadFeaturesTask(
                    packageManager.getPackageInfo(
                        packageName,
                        PackageManager.GET_ACTIVITIES or PackageManager.GET_META_DATA
                    )
                )
                withContext(Dispatchers.Main) {
                    onFeaturesLoaded(features)
                }
            } catch (exception: PackageManager.NameNotFoundException) {
                Timber.e(exception, "Could not resolve package info")
            }
        }
    }

    private fun onFeaturesLoaded(featuresList: List<Feature>?) {
        features = featuresList
        if (featuresList.isNullOrEmpty()) {
            return
        }
        val sections: MutableList<FeatureSectionAdapter.Section> = ArrayList()
        var currentCat = ""
        for (i in features!!.indices) {
            val category = features!![i].category
            if (currentCat != category) {
                sections.add(FeatureSectionAdapter.Section(i, category))
                currentCat = category
            }
        }

        sectionAdapter = FeatureSectionAdapter(
            this,
            R.layout.section_main_layout,
            R.id.section_text,
            FeatureAdapter(features!!)
        )
        sectionAdapter!!.setSections(sections.toTypedArray())
        recyclerView.adapter = sectionAdapter
    }

    private fun startFeature(feature: Feature) {
        val intent = Intent()
        intent.component = ComponentName(packageName, feature.name)
        startActivity(intent)
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        outState.putParcelableArrayList(KEY_STATE_FEATURES, features as ArrayList<Feature>?)
    }

    private fun loadFeaturesTask(vararg p0: PackageInfo?): List<Feature> {
        val features: MutableList<Feature> = ArrayList()
        val app = p0[0]
        val packageName = FeatureOverviewActivity::class.java.`package`!!.name
        val metaDataKey = getString(R.string.category)
        if (app != null) {
            for (info in app.activities) {
                if (info.labelRes != 0 && info.name.startsWith(packageName) &&
                    info.name != FeatureOverviewActivity::class.java.name
                ) {
                    val label = getString(info.labelRes)
                    val description = resolveString(info.descriptionRes)
                    val category = resolveMetaData(info.metaData, metaDataKey)
                    features.add(Feature(info.name, label, description, category!!))
                }
            }
        }
        if (features.isNotEmpty()) {
            val comparator = Comparator { lhs: Feature, rhs: Feature ->
                var result = lhs.category.compareTo(rhs.category, ignoreCase = true)
                if (result == 0) {
                    result = lhs.getLabel().compareTo(rhs.getLabel(), ignoreCase = true)
                }
                result
            }
            Collections.sort(features, comparator)
        }
        return features
    }

    private fun resolveMetaData(bundle: Bundle?, key: String): String? {
        var category: String? = null
        if (bundle != null) {
            category = bundle.getString(key)
        }
        return category
    }

    private fun resolveString(@StringRes stringRes: Int): String {
        return try {
            getString(stringRes)
        } catch (exception: NotFoundException) {
            "-"
        }
    }

    companion object {
        private const val KEY_STATE_FEATURES = "featureList"
    }
}
