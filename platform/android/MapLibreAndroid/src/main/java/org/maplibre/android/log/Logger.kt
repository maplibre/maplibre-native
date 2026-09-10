package org.maplibre.android.log

import android.util.Log
import androidx.annotation.IntDef
import androidx.annotation.Keep

/**
 * Logger for the MapLibre Maps SDK for Android
 *
 * Default implementation relies on [Log].
 * Alternative implementations can be set with [setLoggerDefinition].
 */
@Keep
object Logger {
    /**
     * Priority constant for the println method; use Logger.v
     *
     * This log level will print all logs.
     */
    const val VERBOSE: Int = Log.VERBOSE

    /**
     * Priority constant for the println method; use Logger.d.
     *
     * This log level will print all logs except verbose.
     */
    const val DEBUG: Int = Log.DEBUG

    /**
     * Priority constant for the println method; use Logger.i.
     *
     * This log level will print all logs except verbose and debug.
     */
    const val INFO: Int = Log.INFO

    /**
     * Priority constant for the println method; use Logger.w.
     *
     * This log level will print only warn and error logs.
     */
    const val WARN: Int = Log.WARN

    /**
     * Priority constant for the println method; use Logger.e.
     *
     * This log level will print only error logs.
     */
    const val ERROR: Int = Log.ERROR

    /**
     * Priority constant for the println method.
     *
     * This log level won't print any logs.
     */
    const val NONE: Int = 99

    /**
     * Log level indicates which logs are allowed to be emitted by the MapLibre Maps SDK for Android.
     */
    @IntDef(VERBOSE, DEBUG, INFO, WARN, ERROR, NONE)
    @Retention(AnnotationRetention.SOURCE)
    annotation class LogLevel

    private val DEFAULT: LoggerDefinition =
        object : LoggerDefinition {
            override fun v(
                tag: String,
                msg: String,
            ) {
                Log.v(tag, msg)
            }

            override fun v(
                tag: String,
                msg: String,
                tr: Throwable,
            ) {
                Log.v(tag, msg, tr)
            }

            override fun d(
                tag: String,
                msg: String,
            ) {
                Log.d(tag, msg)
            }

            override fun d(
                tag: String,
                msg: String,
                tr: Throwable,
            ) {
                Log.d(tag, msg, tr)
            }

            override fun i(
                tag: String,
                msg: String,
            ) {
                Log.i(tag, msg)
            }

            override fun i(
                tag: String,
                msg: String,
                tr: Throwable,
            ) {
                Log.i(tag, msg, tr)
            }

            override fun w(
                tag: String,
                msg: String,
            ) {
                Log.w(tag, msg)
            }

            override fun w(
                tag: String,
                msg: String,
                tr: Throwable,
            ) {
                Log.w(tag, msg, tr)
            }

            override fun e(
                tag: String,
                msg: String,
            ) {
                Log.e(tag, msg)
            }

            override fun e(
                tag: String,
                msg: String,
                tr: Throwable,
            ) {
                Log.e(tag, msg, tr)
            }
        }

    @Volatile
    private var logger: LoggerDefinition = DEFAULT

    @LogLevel
    private var logLevel = 0

    /**
     * Set the verbosity of the Logger.
     *
     * This configuration can be used to have more granular control over which logs are emitted by the
     * MapLibre Maps SDK for Android.
     *
     * @param logLevel the verbosity level
     */
    @JvmStatic
    fun setVerbosity(
        @LogLevel logLevel: Int,
    ) {
        Logger.logLevel = logLevel
    }

    /**
     * Replace the current used logger definition.
     *
     * @param loggerDefinition the definition of the logger
     */
    @JvmStatic
    fun setLoggerDefinition(loggerDefinition: LoggerDefinition) {
        logger = loggerDefinition
    }

    /**
     * Send a verbose log message.
     *
     * @param tag Used to identify the source of a log message.  It usually identifies
     *            the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     */
    @JvmStatic
    fun v(
        tag: String,
        msg: String,
    ) {
        if (logLevel <= VERBOSE) {
            logger.v(tag, msg)
        }
    }

    /**
     * Send a verbose log message and log the exception.
     *
     * @param tag Used to identify the source of a log message.  It usually identifies
     *            the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     * @param tr  An exception to log
     */
    @JvmStatic
    fun v(
        tag: String,
        msg: String,
        tr: Throwable,
    ) {
        if (logLevel <= VERBOSE) {
            logger.v(tag, msg, tr)
        }
    }

    /**
     * Send a debug log message.
     *
     * @param tag Used to identify the source of a log message.  It usually identifies
     *            the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     */
    @JvmStatic
    fun d(
        tag: String,
        msg: String,
    ) {
        if (logLevel <= DEBUG) {
            logger.d(tag, msg)
        }
    }

    /**
     * Send a debug log message and log the exception.
     *
     * @param tag Used to identify the source of a log message.  It usually identifies
     *            the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     * @param tr  An exception to log
     */
    @JvmStatic
    fun d(
        tag: String,
        msg: String,
        tr: Throwable,
    ) {
        if (logLevel <= DEBUG) {
            logger.d(tag, msg, tr)
        }
    }

    /**
     * Send an info log message.
     *
     * @param tag Used to identify the source of a log message.  It usually identifies
     *            the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     */
    @JvmStatic
    fun i(
        tag: String,
        msg: String,
    ) {
        if (logLevel <= INFO) {
            logger.i(tag, msg)
        }
    }

    /**
     * Send an info log message and log the exception.
     *
     * @param tag Used to identify the source of a log message.  It usually identifies
     *            the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     * @param tr  An exception to log
     */
    @JvmStatic
    fun i(
        tag: String,
        msg: String,
        tr: Throwable,
    ) {
        if (logLevel <= INFO) {
            logger.i(tag, msg, tr)
        }
    }

    /**
     * Send a warning log message.
     *
     * @param tag Used to identify the source of a log message.  It usually identifies
     *            the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     */
    @JvmStatic
    fun w(
        tag: String,
        msg: String,
    ) {
        if (logLevel <= WARN) {
            logger.w(tag, msg)
        }
    }

    /**
     * Send a warning log message and log the exception.
     *
     * @param tag Used to identify the source of a log message.  It usually identifies
     *            the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     * @param tr  An exception to log
     */
    @JvmStatic
    fun w(
        tag: String,
        msg: String,
        tr: Throwable,
    ) {
        if (logLevel <= WARN) {
            logger.w(tag, msg, tr)
        }
    }

    /**
     * Send an error log message.
     *
     * @param tag Used to identify the source of a log message.  It usually identifies
     *            the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     */
    @JvmStatic
    fun e(
        tag: String,
        msg: String,
    ) {
        if (logLevel <= ERROR) {
            logger.e(tag, msg)
        }
    }

    /**
     * Send an error log message and log the exception.
     *
     * @param tag Used to identify the source of a log message.  It usually identifies
     *            the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     * @param tr  An exception to log
     */
    @JvmStatic
    fun e(
        tag: String,
        msg: String,
        tr: Throwable,
    ) {
        if (logLevel <= ERROR) {
            logger.e(tag, msg, tr)
        }
    }

    /**
     * Send a log message based on severity.
     *
     * @param severity the log severity
     * @param tag      Used to identify the source of a log message.  It usually identifies
     *                 the class or activity where the log call occurs.
     * @param message  The message you would like logged.
     */
    @JvmStatic
    fun log(
        severity: Int,
        tag: String,
        message: String,
    ) {
        when (severity) {
            Log.VERBOSE -> v(tag, message)
            Log.DEBUG -> d(tag, message)
            Log.INFO -> i(tag, message)
            Log.WARN -> w(tag, message)
            Log.ERROR -> e(tag, message)
            else -> throw UnsupportedOperationException()
        }
    }
}
