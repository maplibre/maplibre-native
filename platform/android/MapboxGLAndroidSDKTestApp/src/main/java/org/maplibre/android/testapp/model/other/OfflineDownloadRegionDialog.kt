package org.maplibre.android.testapp.model.other

import android.app.Activity
import android.app.Dialog
import android.content.DialogInterface
import android.os.Bundle
import android.widget.EditText
import androidx.appcompat.app.AlertDialog
import androidx.fragment.app.DialogFragment
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.model.other.OfflineDownloadRegionDialog.DownloadRegionDialogListener
import timber.log.Timber

class OfflineDownloadRegionDialog : DialogFragment() {
    interface DownloadRegionDialogListener {
        fun onDownloadRegionDialogPositiveClick(regionName: String?)
    }

    var listener: DownloadRegionDialogListener? = null
    override fun onAttach(activity: Activity) {
        super.onAttach(activity)
        listener = activity as DownloadRegionDialogListener
    }

    override fun onCreateDialog(savedInstanceState: Bundle?): Dialog {
        val builder = AlertDialog.Builder(
            requireActivity()
        )

        // Let the user choose a name for the region
        val regionNameEdit = EditText(activity)
        builder.setTitle("Choose a name for the region")
            .setIcon(R.drawable.ic_airplanemode_active_black)
            .setView(regionNameEdit)
            .setPositiveButton("Start") { dialog: DialogInterface?, which: Int ->
                val regionName = regionNameEdit.text.toString()
                listener!!.onDownloadRegionDialogPositiveClick(regionName)
            }
            .setNegativeButton("Cancel") { dialog: DialogInterface?, which: Int -> Timber.d("Download cancelled.") }
        return builder.create()
    }
}
