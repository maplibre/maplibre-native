package org.maplibre.android.utils

import android.os.AsyncTask
import org.maplibre.android.log.Logger
import java.io.File

object FileUtils {
    private const val TAG = "Mbgl-FileUtils"

    /**
     * Task checking whether app's process can read a file.
     *
     * The callback reference is **strongly kept** throughout the process,
     * so it needs to be wrapped in a weak reference or released on the client side if necessary.
     */
    @Suppress("DEPRECATION")
    class CheckFileReadPermissionTask(
        private val listener: OnCheckFileReadPermissionListener,
    ) : AsyncTask<File, Void, Boolean>() {
        override fun doInBackground(vararg files: File): Boolean =
            try {
                files[0].canRead()
            } catch (ex: Exception) {
                false
            }

        override fun onCancelled() {
            listener.onError()
        }

        override fun onPostExecute(result: Boolean) {
            if (result) {
                listener.onReadPermissionGranted()
            } else {
                listener.onError()
            }
        }
    }

    /**
     * Interface definition for a callback invoked when checking file's read permissions.
     */
    interface OnCheckFileReadPermissionListener {
        /**
         * Invoked when app's process has a permission to read a file.
         */
        fun onReadPermissionGranted()

        /**
         * Invoked when app's process doesn't have a permission to read a file or an error occurs.
         */
        fun onError()
    }

    /**
     * Task checking whether app's process can write to a file.
     *
     * The callback reference is **strongly kept** throughout the process,
     * so it needs to be wrapped in a weak reference or released on the client side if necessary.
     */
    @Suppress("DEPRECATION")
    class CheckFileWritePermissionTask(
        private val listener: OnCheckFileWritePermissionListener,
    ) : AsyncTask<File, Void, Boolean>() {
        override fun doInBackground(vararg files: File): Boolean =
            try {
                files[0].canWrite()
            } catch (ex: Exception) {
                false
            }

        override fun onCancelled() {
            listener.onError()
        }

        override fun onPostExecute(result: Boolean) {
            if (result) {
                listener.onWritePermissionGranted()
            } else {
                listener.onError()
            }
        }
    }

    /**
     * Interface definition for a callback invoked when checking file's write permissions.
     */
    interface OnCheckFileWritePermissionListener {
        /**
         * Invoked when app's process has a permission to write to a file.
         */
        fun onWritePermissionGranted()

        /**
         * Invoked when app's process doesn't have a permission to write to a file or an error occurs.
         */
        fun onError()
    }

    /**
     * Deletes a file asynchronously in a separate thread.
     *
     * @param path the path of the file that should be deleted
     */
    @JvmStatic
    fun deleteFile(path: String) {
        // Delete the file in a separate thread to avoid affecting the UI
        Thread {
            try {
                val file = File(path)
                if (file.exists()) {
                    if (file.delete()) {
                        Logger.d(TAG, "File deleted to save space: $path")
                    } else {
                        Logger.e(TAG, "Failed to delete file: $path")
                    }
                }
            } catch (exception: Exception) {
                Logger.e(TAG, "Failed to delete file: ", exception)
            }
        }.start()
    }
}
