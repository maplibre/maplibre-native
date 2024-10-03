import java.net.*;


// Get the validation layer version.
var VVL_VER = "1.3.290.0"
if (extra.has("vvl_version")) {
    VVL_VER = extra["vvl_version"] as String
}

// Declare local variables shared between downloading and unzipping.
val VVL_SITE = "https://github.com/KhronosGroup/Vulkan-ValidationLayers"
val VVL_LIB_ROOT = projectDir // or rootDir
val VVL_JNILIB_DIR = "$VVL_LIB_ROOT/src/vulkanDebug/jniLibs"
val VVL_SO_NAME = "libVkLayer_khronos_validation.so"

// Download the release zip file to ${VVL_LIB_ROOT}/
tasks.register("download") {
    val VVL_ZIP_NAME = "releases/download/vulkan-sdk-$VVL_VER/android-binaries-$VVL_VER.zip"
    mkdir(VVL_LIB_ROOT)
    val f = file("$VVL_LIB_ROOT/android-binaries-$VVL_VER.zip")
    doLast {
        URL("$VVL_SITE/$VVL_ZIP_NAME").openStream().use { inputStream ->
            f.outputStream().use { outputStream ->
                inputStream.copyTo(outputStream)
            }
        }
    }
}

// Unzip the downloaded VVL zip archive to the ${VVL_JNILIB_DIR} for APK packaging.
tasks.register<Copy>("unzip") {
    dependsOn("download")
    from(zipTree(file("$VVL_LIB_ROOT/android-binaries-$VVL_VER.zip")))
    into(file("$VVL_JNILIB_DIR"))
}

// Configure the Android source set for JNI libs.
//android.sourceSets["vulkan"].jniLibs {
//    srcDir(tasks.named("unzip").get().outputs.files)
//}