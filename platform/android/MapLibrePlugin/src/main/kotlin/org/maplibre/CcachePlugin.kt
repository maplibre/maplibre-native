package org.maplibre

import com.android.build.api.dsl.ApplicationExtension
import com.android.build.api.dsl.LibraryExtension
import org.gradle.api.Plugin
import org.gradle.api.Project
import java.io.File

private fun configureCcacheArguments(arguments: MutableList<String>) {
    val cacheToolPath = findCacheToolPath()
    if (cacheToolPath != null) {
        arguments.add("-DCMAKE_CXX_COMPILER_LAUNCHER=$cacheToolPath")
        println("ccache enabled at: $cacheToolPath")
    } else {
        println("ccache not found on the system, continuing without it.")
    }
}

fun configureCcache(project: Project) {
    val appExtension = project.extensions.findByType(ApplicationExtension::class.java)
    if (appExtension != null) {
        appExtension.defaultConfig {
            externalNativeBuild {
                cmake {
                    configureCcacheArguments(arguments)
                }
            }
        }
        return
    }

    val libraryExtension = project.extensions.findByType(LibraryExtension::class.java)
    if (libraryExtension != null) {
        libraryExtension.defaultConfig {
            externalNativeBuild {
                cmake {
                    configureCcacheArguments(arguments)
                }
            }
        }
    }
}

fun findCacheToolPath(): String? {
    val os = System.getProperty("os.name")
    val ccacheCommand = if (os.startsWith("Windows")) "where ccache" else "which ccache"
    val sccacheCommand = if (os.startsWith("Windows")) "where sccache" else "which sccache"

    fun findCommandPath(command: String): String? {
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

    // Try to find sccache first, then fallback to ccache
    return findCommandPath(sccacheCommand) ?: findCommandPath(ccacheCommand)
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
