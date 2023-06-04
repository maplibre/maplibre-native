package org.maplibre.android.testapp.model.activity

import android.os.Parcel
import android.os.Parcelable

class Feature : Parcelable {
    var name: String
        private set
    private var label: String?
    private var description: String?
    var category: String
        private set

    constructor(name: String, label: String?, description: String?, category: String) {
        this.name = name
        this.label = label
        this.description = description
        this.category = category
    }

    private constructor(`in`: Parcel) {
        name = `in`.readString().toString()
        label = `in`.readString()
        description = `in`.readString()
        category = `in`.readString().toString()
    }

    val simpleName: String
        get() {
            val split = name.split("\\.").toTypedArray()
            return split[split.size - 1]
        }

    fun getLabel(): String {
        return if (label != null) label!! else simpleName
    }

    fun getDescription(): String {
        return if (description != null) description!! else "-"
    }

    override fun describeContents(): Int {
        return 0
    }

    override fun writeToParcel(out: Parcel, flags: Int) {
        out.writeString(name)
        out.writeString(label)
        out.writeString(description)
        out.writeString(category)
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<Feature?> = object : Parcelable.Creator<Feature?> {
            override fun createFromParcel(`in`: Parcel): Feature {
                return Feature(`in`)
            }

            override fun newArray(size: Int): Array<Feature?> {
                return arrayOfNulls(size)
            }
        }
    }
}
