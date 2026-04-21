import com.android.build.api.dsl.ApplicationExtension
import com.android.build.api.dsl.LibraryExtension
import org.gradle.api.Plugin
import org.gradle.api.Project


open class DownloadVulkanValidationPlugin : Plugin<Project> {
    override fun apply(project: Project) {
        project.configureVulkanJniLibs()
    }
}

internal fun Project.configureVulkanJniLibs() {
    val appExtension = extensions.findByType(ApplicationExtension::class.java)
    if (appExtension != null) {
        appExtension.sourceSets {
            getByName("vulkan") {
                jniLibs.srcDir(tasks.named("unzip").get().outputs.files.asFileTree)
            }
        }
        return
    }

    val libraryExtension = extensions.findByType(LibraryExtension::class.java)
    if (libraryExtension != null) {
        libraryExtension.sourceSets {
            getByName("vulkan") {
                jniLibs.srcDir(tasks.named("unzip").get().outputs.files.asFileTree)
            }
        }
    }
}
