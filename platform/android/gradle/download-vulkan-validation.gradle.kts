// Get the validation layer version.
val VVL_VER: String = if (extra.has("vvl_version")) {
    extra["vvl_version"].toString()
} else {
    "1.3.290.0"
}

// Declare local variables shared between downloading and unzipping.
val VVL_SITE = "https://github.com/KhronosGroup/Vulkan-ValidationLayers"
val VVL_LIB_ROOT = projectDir.toString()
val VVL_JNILIB_DIR = "${VVL_LIB_ROOT}/src/vulkanDebug/jniLibs"
val VVL_SO_NAME = "libVkLayer_khronos_validation.so"

// Download the release zip file to ${VVL_LIB_ROOT}/
tasks.register("download") {
    doLast {
        val VVL_ZIP_NAME = "releases/download/vulkan-sdk-$VVL_VER/android-binaries-$VVL_VER.zip"
        mkdir(VVL_LIB_ROOT)
        val file = file("${VVL_LIB_ROOT}/android-binaries-$VVL_VER.zip")
        URL("${VVL_SITE}/${VVL_ZIP_NAME}").openStream().use { input ->
            file.outputStream().use { output -> input.copyTo(output) }
        }
    }
}

// Unzip the downloaded VVL zip archive to the ${VVL_JNILIB_DIR} for APK packaging.
tasks.register<Copy>("unzip") {
    dependsOn("download")
    from(zipTree(file("${VVL_LIB_ROOT}/android-binaries-$VVL_VER.zip")))
    into(file(VVL_JNILIB_DIR))
}

// Set the sourceSets to include the unzipped JNI libraries
android.sourceSets.getByName("vulkan").jniLibs.srcDir(tasks.named("unzip"))
