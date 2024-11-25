package org.maplibre

import com.android.build.gradle.BaseExtension
import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.kotlin.dsl.getByType
import java.io.File

fun configureCcache(project: Project) {
    project.extensions.getByType<BaseExtension>().run {
        defaultConfig {
            externalNativeBuild {
                cmake {
                    val ccachePath = findCcachePath()
                    if (ccachePath != null) {
                        arguments.add("-DCMAKE_CXX_COMPILER_LAUNCHER=$ccachePath")
                        println("ccache enabled at: $ccachePath")
                    } else {
                        println("ccache not found on the system, continuing without it.")
                    }
                }
            }
        }
    }
}

fun findCcachePath(): String? {
    val command = if (System.getProperty("os.name").startsWith("Windows")) {
        "where sccache"
    } else {
        "which ccache"
    }

    return try {
        val process = Runtime.getRuntime().exec(command)
        val result = process.inputStream.bufferedReader().readText().trim()
        if (process.waitFor() == 0 && result.isNotEmpty()) {
            File(result).absolutePath
        } else {
            null
        }
    } catch (e: Exception) {
        null
    }
}

class CcachePlugin : Plugin<Project> {
    override fun apply(project: Project) {
        // Apply only when the appropriate Android plugin is present
        project.plugins.withId("com.android.application") {
            configureCcache(project)
        }
        project.plugins.withId("com.android.library") {
            configureCcache(project)
        }
    }
}

