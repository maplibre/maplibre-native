import java.net.URL


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
var vvlVersion = "1.4.313.0"
if (extra.has("vvl_version")) {
    vvlVersion = extra["vvl_version"] as String
}

// Declare variables for the Vulkan Validation Layers download and extraction paths.
val vvlSite = "https://github.com/KhronosGroup/Vulkan-ValidationLayers"
val vvlLibRoot = projectDir // Set project root or change to rootDir if needed
val vvlJniLibDir = "$vvlLibRoot/src/vulkanDebug/jniLibs"
val vvlSoName = "libVkLayer_khronos_validation.so"

// Download the release zip file to ${vvlLibRoot}/
val download = tasks.register("download") {
    val vvlZipName = "releases/download/vulkan-sdk-$vvlVersion/android-binaries-$vvlVersion.zip"
    val zipFile = file("$vvlLibRoot/android-binaries-$vvlVersion.zip")

    mkdir(vvlLibRoot)

    doLast {
        // Download the zip file from the Vulkan Validation Layers GitHub repo.
        URL("$vvlSite/$vvlZipName").openStream().use { inputStream ->
            zipFile.outputStream().use { outputStream ->
                inputStream.copyTo(outputStream)
            }
        }
    }
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
