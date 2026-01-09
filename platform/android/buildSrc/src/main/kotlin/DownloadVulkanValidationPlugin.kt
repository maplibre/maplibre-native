import com.android.build.gradle.BaseExtension
import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.kotlin.dsl.getByType


open class DownloadVulkanValidationPlugin : Plugin<Project> {
    override fun apply(project: Project) {
        project.configureVulkanJniLibs()
    }
}

internal fun Project.configureVulkanJniLibs() = this.extensions.getByType<BaseExtension>().run {
    sourceSets {
        getByName("vulkan") {
            jniLibs.srcDir(tasks.named("unzip").get().outputs.files.asFileTree)
        }
    }
}
