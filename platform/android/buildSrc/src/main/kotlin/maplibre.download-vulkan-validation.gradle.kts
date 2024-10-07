import java.net.URL

// Define the validation layer version, with a default value.
var vvlVersion = "1.3.290.0"
if (extra.has("vvl_version")) {
    vvlVersion = extra["vvl_version"] as String
}

// Declare variables for the Vulkan Validation Layers download and extraction paths.
val vvlSite = "https://github.com/KhronosGroup/Vulkan-ValidationLayers"
val vvlLibRoot = projectDir // Set project root or change to rootDir if needed
val vvlJniLibDir = "$vvlLibRoot/src/vulkanDebug/jniLibs"
val vvlSoName = "libVkLayer_khronos_validation.so"

// Download the release zip file to ${vvlLibRoot}/
tasks.register("download") {
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

// Unzip the downloaded VVL zip archive to the ${vvlJniLibDir} for APK packaging.
tasks.register<Copy>("unzip") {
    dependsOn("download")

    from(zipTree(file("$vvlLibRoot/android-binaries-$vvlVersion.zip")))
    into(file(vvlJniLibDir))
}