import java.net.URI


/**
 * Download validation layer binary release zip file from Khronos github repo
 *    https://github.com/KhronosGroup/Vulkan-ValidationLayers/releases
 *
 * To use this script, add the following to your module's build.gradle:
 *     ext.vvl_version='your-new-version'
 *     apply from: "${PATH-TO-THIS}/download_vvl.gradle"
 * To update to a new version:
 *   - change the ext.vvl_version to a new version string.
 *   - delete directory pointed by ${vvlJniLibDir}.
 *   - sync gradle script in IDE and rebuild project.
 *
 * Note: binary release can also be manually downloaded and put it into
 *       the default jniLibs directory at app/src/main/jniLibs.
 */

// Define the validation layer version, with a default value.
var vvlVersion = "1.4.350.0"
if (extra.has("vvl_version")) {
    vvlVersion = extra["vvl_version"] as String
}

// Declare variables for the Vulkan Validation Layers download and extraction paths.
val vvlSite = "https://github.com/KhronosGroup/Vulkan-ValidationLayers"
val vvlLibRoot = projectDir // Set project root or change to rootDir if needed
val vvlJniLibDir = "$vvlLibRoot/src/vulkanDebug/jniLibs"
val vvlSoName = "libVkLayer_khronos_validation.so"

abstract class Download : DefaultTask() {
    @get:Input
    abstract val url: Property<String>

    @get:OutputFile
    abstract val file: RegularFileProperty

    @TaskAction
    fun download() {
        URI.create(url.get()).toURL().openStream().use { inputStream ->
            file.get().asFile
                .outputStream()
                .use { outputStream ->
                    inputStream.copyTo(outputStream)
                }
        }
    }
}

val download = tasks.register<Download>("download") {
    // Download the zip file from the Vulkan Validation Layers GitHub repo.
    url = "$vvlSite/releases/download/vulkan-sdk-$vvlVersion/android-binaries-$vvlVersion.zip"
    // Download the release zip file to ${vvlLibRoot}/
    file = file("$vvlLibRoot/android-binaries-$vvlVersion.zip")
}

val unzip = tasks.register<Copy>("unzip") {
    dependsOn(download)

    from(zipTree(file("$vvlLibRoot/android-binaries-$vvlVersion.zip"))) {
        eachFile {
            path = path.substringAfter("/")
        }
        includeEmptyDirs = false // Optional: Exclude empty directories if not needed
    }
    into(file(vvlJniLibDir))

    doFirst {
        mkdir(vvlJniLibDir)
    }
}

tasks.named("preBuild") {
    dependsOn(unzip)
}
