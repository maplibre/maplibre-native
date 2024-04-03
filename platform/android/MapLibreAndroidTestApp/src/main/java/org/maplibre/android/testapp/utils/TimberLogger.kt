package org.maplibre.android.testapp.utils

import org.maplibre.android.log.LoggerDefinition
import timber.log.Timber

class TimberLogger : LoggerDefinition {
    override fun v(tag: String, msg: String) {
        Timber.tag(tag).v(msg)
    }

    override fun v(tag: String, msg: String, tr: Throwable) {
        Timber.tag(tag).v(tr, msg)
    }

    override fun d(tag: String, msg: String) {
        Timber.tag(tag).d(msg)
    }

    override fun d(tag: String, msg: String, tr: Throwable) {
        Timber.tag(tag).d(tr, msg)
    }

    override fun i(tag: String, msg: String) {
        Timber.tag(tag).i(msg)
    }

    override fun i(tag: String, msg: String, tr: Throwable) {
        Timber.tag(tag).i(tr, msg)
    }

    override fun w(tag: String, msg: String) {
        Timber.tag(tag).w(msg)
    }

    override fun w(tag: String, msg: String, tr: Throwable) {
        Timber.tag(tag).w(tr, msg)
    }

    override fun e(tag: String, msg: String) {
        Timber.tag(tag).e(msg)
    }

    override fun e(tag: String, msg: String, tr: Throwable) {
        Timber.tag(tag).e(tr, msg)
    }
}
