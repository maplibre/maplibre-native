plugins {
    id("org.gradle.toolchains.foojay-resolver-convention") version "0.8.0"
}

include(":MapLibreAndroid", ":MapLibreAndroidTestApp", ":MapLibreAndroidLint")

rootProject.name = "MapLibre Native for Android"

val renderTestProjectDir = File(rootDir, "../../render-test/android")
includeBuild(renderTestProjectDir) {
    name = "renderTestApp"
}

val cppTestProjectDir = File(rootDir, "../../test/android")
includeBuild(cppTestProjectDir) {
    name = "cppUnitTestsApp"
}
