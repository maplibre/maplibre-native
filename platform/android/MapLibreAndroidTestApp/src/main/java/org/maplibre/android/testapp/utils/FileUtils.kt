package org.maplibre.android.testapp.utils

import android.content.Context
import java.io.File
import java.io.FileOutputStream

object FileUtils {

    /**
     * Task that copies a file from the assets directory to a provided directory.
     * The asset's name is going to be kept in the new directory.
     */
    fun copyFileFromAssetsTask(
        context: Context,
        assetName: String,
        destinationPath: String
    ) : Boolean =
        try {
            copyAsset(context, assetName, destinationPath)
            true
        } catch (ex: Exception) {
            false
        }

    private fun copyAsset(context: Context, assetName: String, destinationPath: String) {
        val bufferSize = 1024
        val assetManager = context.assets
        val inputStream = assetManager.open(assetName)
        val outputStream = FileOutputStream(File(destinationPath, assetName))
        try {
            inputStream.copyTo(outputStream, bufferSize)
        } finally {
            inputStream.close()
            outputStream.flush()
            outputStream.close()
        }
    }
}
