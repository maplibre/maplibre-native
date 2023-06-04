package org.maplibre.android.testapp.utils

import android.os.SystemClock
import timber.log.Timber
import java.util.ArrayList

/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */ /**
 * A utility class to help log timings splits throughout a method call.
 * Typical usage is:
 *
 *
 * <pre>
 * TimingLogger timings = new TimingLogger(TAG, "methodA");
 * // ... do some work A ...
 * timings.addSplit("work A");
 * // ... do some work B ...
 * timings.addSplit("work B");
 * // ... do some work C ...
 * timings.addSplit("work C");
 * timings.dumpToLog();
</pre> *
 *
 *
 *
 * The dumpToLog call would add the following to the log:
 *
 *
 * <pre>
 * D/TAG     ( 3459): methodA: begin
 * D/TAG     ( 3459): methodA:      9 ms, work A
 * D/TAG     ( 3459): methodA:      1 ms, work B
 * D/TAG     ( 3459): methodA:      6 ms, work C
 * D/TAG     ( 3459): methodA: end, 16 ms
</pre> *
 */
class TimingLogger(tag: String?, label: String?) {
    /**
     * The Log tag to use for checking Log.isLoggable and for
     * logging the timings.
     */
    private var tag: String? = null

    /**
     * A label to be included in every log.
     */
    private var label: String? = null

    /**
     * Used to track whether Log.isLoggable was enabled at reset time.
     */
    private var disabled = false

    /**
     * Stores the time of each split.
     */
    private var splits: ArrayList<Long>? = null

    /**
     * Stores the labels for each split.
     */
    private var splitLabels: ArrayList<String?>? = null

    /**
     * Clear and initialize a TimingLogger object that will log using
     * the specific tag. If the Log.isLoggable is not enabled to at
     * least the Log.VERBOSE level for that tag at creation time then
     * the addSplit and dumpToLog call will do nothing.
     *
     * @param tag   the log tag to use while logging the timings
     * @param label a string to be displayed with each log
     */
    fun reset(tag: String?, label: String?) {
        this.tag = tag
        this.label = label
        reset()
    }

    /**
     * Clear and initialize a TimingLogger object that will log using
     * the tag and label that was specified previously, either via
     * the constructor or a call to reset(tag, label). If the
     * Log.isLoggable is not enabled to at least the Log.VERBOSE
     * level for that tag at creation time then the addSplit and
     * dumpToLog call will do nothing.
     */
    fun reset() {
        disabled = false // !Log.isLoggable(tag, Log.VERBOSE);
        if (disabled) {
            return
        }
        if (splits == null) {
            splits = ArrayList()
            splitLabels = ArrayList()
        } else {
            splits!!.clear()
            splitLabels!!.clear()
        }
        addSplit(null)
    }

    /**
     * Add a split for the current time, labeled with splitLabel. If
     * Log.isLoggable was not enabled to at least the Log.VERBOSE for
     * the specified tag at construction or reset() time then this
     * call does nothing.
     *
     * @param splitLabel a label to associate with this split.
     */
    fun addSplit(splitLabel: String?) {
        if (disabled) {
            return
        }
        val now = SystemClock.elapsedRealtime()
        splits!!.add(now)
        splitLabels!!.add(splitLabel)
    }

    /**
     * Dumps the timings to the log using Timber.d(). If Log.isLoggable was
     * not enabled to at least the Log.VERBOSE for the specified tag at
     * construction or reset() time then this call does nothing.
     */
    fun dumpToLog() {
        if (disabled) {
            return
        }
        Timber.d("%s: begin", label)
        val first = splits!![0]
        var now = first
        for (i in 1 until splits!!.size) {
            now = splits!![i]
            val splitLabel = splitLabels!![i]
            val prev = splits!![i - 1]
            Timber.d("%s:      %s ms, %s", label, now - prev, splitLabel)
        }
        Timber.d("%s: end, %s ms", label, now - first)
    }

    /**
     * Create and initialize a TimingLogger object that will log using
     * the specific tag. If the Log.isLoggable is not enabled to at
     * least the Log.VERBOSE level for that tag at creation time then
     * the addSplit and dumpToLog call will do nothing.
     *
     * @param tag   the log tag to use while logging the timings
     * @param label a string to be displayed with each log
     */
    init {
        reset(tag, label)
    }
}
