package org.maplibre.android.testapp.model.other

import android.app.Dialog
import android.content.DialogInterface
import android.os.Bundle
import androidx.appcompat.app.AlertDialog
import androidx.fragment.app.DialogFragment
import org.maplibre.android.testapp.R
import timber.log.Timber

class OfflineListRegionsDialog : DialogFragment() {
    override fun onCreateDialog(savedInstanceState: Bundle?): Dialog {
        val builder = AlertDialog.Builder(
            requireActivity()
        )

        // Read args
        val args = arguments
        val offlineRegionsNames = args?.getStringArrayList(ITEMS)
        val items = offlineRegionsNames!!.toTypedArray<CharSequence>()
        builder.setTitle("List of offline regions")
            .setIcon(R.drawable.ic_airplanemode_active_black)
            .setItems(items) { dialog: DialogInterface?, which: Int ->
                Timber.d(
                    "Selected item: %s",
                    which
                )
            }
            .setPositiveButton("Accept") { dialog: DialogInterface?, which: Int -> Timber.d("Dialog dismissed") }
        return builder.create()
    }

    companion object {
        const val ITEMS = "ITEMS"
    }
}
