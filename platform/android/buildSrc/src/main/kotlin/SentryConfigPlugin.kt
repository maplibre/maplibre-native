import com.android.build.gradle.AppExtension
import java.io.ByteArrayOutputStream
import org.gradle.api.Plugin
import org.gradle.api.Project

class SentryConditionalPlugin : Plugin<Project> {
    override fun apply(project: Project) {
        val sentryProject = System.getenv("SENTRY_PROJECT")

        if (sentryProject.isNullOrBlank()) {
            println("SENTRY_PROJECT environment variable not set. Skipping Sentry plugin application.")
            return
        }

        // Apply the Sentry plugin via plugin id
        val sentryPluginId = "io.sentry.android.gradle"
        try {
            project.pluginManager.apply(sentryPluginId)
        } catch (t: Throwable) {
            project.logger.warn("Could not apply Sentry plugin ($sentryPluginId): ${t.message}")
            return
        }

        // Configure Sentry extension using reflection to avoid compile-time dependency
        val sentryExt = project.extensions.findByName("sentry")
        if (sentryExt == null) {
            project.logger.warn("Sentry extension not found after applying the plugin. Skipping configuration.")
            return
        }

        fun setProperty(ext: Any, propertyGetterName: String, value: Any?) {
            try {
                val getter = ext.javaClass.methods.firstOrNull { it.name == propertyGetterName && it.parameterCount == 0 }
                    ?: return
                val propObj = getter.invoke(ext) ?: return
                val setMethod = propObj.javaClass.methods.firstOrNull { it.name == "set" && it.parameterCount == 1 }
                    ?: return
                setMethod.invoke(propObj, value)
            } catch (_: Throwable) {
                // Ignore if property isn't present on current plugin version
            }
        }

        val androidExtension = project.extensions.getByType(AppExtension::class.java)

        val propertiesToSet = mapOf(
            "getDebug" to true,
            "getOrg" to System.getenv("SENTRY_ORG"),
            "getProjectName" to System.getenv("SENTRY_PROJECT"),
            "getAuthToken" to System.getenv("SENTRY_AUTH_TOKEN"),
            "getRelease" to androidExtension.defaultConfig.versionName,
            "getIncludeProguardMapping" to true,
            "getAutoUploadProguardMapping" to true,
            "getDexguardEnabled" to false,
            "getUploadNativeSymbols" to true,
            "getAutoUploadNativeSymbols" to false,
            "getIncludeNativeSources" to true,
            "getIncludeSourceContext" to true,
            "getIncludeDependenciesReport" to true,
            "getTelemetry" to false
        )

        propertiesToSet.forEach { (getterName, value) ->
            setProperty(sentryExt, getterName, value)
        }

        project.afterEvaluate {
            val commitHash = ByteArrayOutputStream()

            project.exec {
                commandLine("git", "rev-parse", "HEAD")
                standardOutput = commitHash
                workingDir = project.rootDir
                isIgnoreExitValue = true
            }

            androidExtension.applicationVariants.forEach { variant ->
                variant.mergedFlavor.manifestPlaceholders["SENTRY_ENV"] = "${variant.name}-$commitHash"
            }
        }
    }
}
