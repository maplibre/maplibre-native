import org.jetbrains.dokka.gradle.DokkaTask

plugins {
    alias(libs.plugins.kotlinter)
    alias(libs.plugins.dokka)
}

apply(plugin = "com.android.library")
apply(plugin = "com.jaredsburrows.license")
apply(plugin = "kotlin-android")
apply(from = "${rootDir}/gradle/native-build.gradle.kts")

dependencies {
    lintChecks(project(":MapLibreAndroidLint"))
    api(libs.maplibreJavaGeoJSON)
    api(libs.maplibreGestures)

    implementation(libs.maplibreJavaTurf)
    implementation(libs.supportAnnotations)
    implementation(libs.supportFragmentV4)
    implementation(libs.okhttp3)
    implementation(libs.timber)
    implementation(libs.interpolator)

    testImplementation(libs.junit)
    testImplementation(libs.mockito)
    testImplementation(libs.mockk)
    testImplementation(libs.robolectric)
    testImplementation(libs.commonsIO)
    testImplementation(libs.assertjcore)

    androidTestImplementation(libs.testRunner)
    androidTestImplementation(libs.testRules)
}

tasks.withType<DokkaTask> {
    moduleName.set("MapLibre Native Android")

    dokkaSourceSets.configureEach {
        includes.from("Module.md")
    }
}

android {
    defaultConfig {
        compileSdk = 34
        minSdk = 21
        targetSdk = 33
        buildConfigField("String", "GIT_REVISION_SHORT", "\"${getGitRevision()}\"")
        buildConfigField("String", "GIT_REVISION", "\"${getGitRevision(false)}\"")
        buildConfigField("String", "MAPLIBRE_VERSION_STRING", "\"MapLibre Native/${project.version}\"")
        consumerProguardFiles("proguard-rules.pro")

        externalNativeBuild {
            cmake {
                arguments("-DMLN_LEGACY_RENDERER=ON", "-DMLN_DRAWABLE_RENDERER=OFF")
            }
        }
    }

    flavorDimensions += "renderer"
    productFlavors {
        create("legacy") {
            dimension = "renderer"
            externalNativeBuild {
                cmake {
                    arguments("-DMLN_LEGACY_RENDERER=ON", "-DMLN_DRAWABLE_RENDERER=OFF")
                }
            }
        }
        create("drawable") {
            dimension = "renderer"
            externalNativeBuild {
                cmake {
                    arguments("-DMLN_LEGACY_RENDERER=OFF", "-DMLN_DRAWABLE_RENDERER=ON")
                }
            }
        }
        create("vulkan") {
            dimension = "renderer"
            externalNativeBuild {
                cmake {
                    arguments("-DMLN_LEGACY_RENDERER=OFF", "-DMLN_DRAWABLE_RENDERER=ON")
                    arguments("-DMLN_WITH_OPENGL=OFF", "-DMLN_WITH_VULKAN=ON")
                }
            }
        }
    }

    sourceSets {
        getByName("legacy").java.srcDirs("src/opengl/java/")
        getByName("drawable").java.srcDirs("src/opengl/java/")
    }

    val nativeTargets = mutableListOf("maplibre")
    if (project.hasProperty("mapbox.with_test")) {
        nativeTargets.add("mbgl-test")
    }
    if (project.hasProperty("mapbox.with_benchmark")) {
        nativeTargets.add("mbgl-benchmark")
    }
    nativeBuild(nativeTargets)

    resourcePrefix("maplibre_")

    sourceSets {
        getByName("main").res.srcDirs("src/main/res-public")
    }

    testOptions {
        unitTests.isReturnDefaultValues = true
        unitTests.isIncludeAndroidResources = true
    }

    buildTypes {
        getByName("debug") {
            isTestCoverageEnabled = false
            isJniDebuggable = true
            isDebuggable = true
        }
    }

    namespace = "org.maplibre.android"
    lint {
        checkAllWarnings = true
        disable.addAll(listOf("MissingTranslation", "TypographyQuotes", "ObsoleteLintCustomCheck", "MissingPermission", "WrongThreadInterprocedural"))
        warningsAsErrors = false
    }

    buildFeatures {
        buildConfig = true
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }

    kotlinOptions {
        jvmTarget = "11"
    }
}

licenseReport {
    generateHtmlReport = false
    generateJsonReport = true
    copyHtmlReportToAssets = false
    copyJsonReportToAssets = false
}

fun getGitRevision(shortRev: Boolean = true): String {
    val cmd = if (shortRev) "git rev-parse --short HEAD" else "git rev-parse HEAD"
    return cmd.runCommand().trim()
}

configurations.all {
    exclude(group = "commons-logging", module = "commons-logging")
    exclude(group = "commons-collections", module = "commons-collections")
}
/ Download validation layers from
// https://github.com/KhronosGroup/Vulkan-ValidationLayers

apply(from = "${rootDir}/gradle/download-vulkan-validation.gradle.kts")
apply(from = "${rootDir}/gradle/gradle-checkstyle.gradle.kts")
apply(from = "${rootDir}/gradle/gradle-dependencies-graph.gradle.kts")
apply(from = "${rootDir}/gradle/android-nitpick.gradle.kts")
apply(from = "${rootDir}/gradle/gradle-publish.gradle.kts")
// intentionally disabled
//apply(from = "${rootDir}/gradle/jacoco-report.gradle.kts")


// Helper extension to execute shell commands
fun String.runCommand(): String {
    return ProcessBuilder(*split(" ").toTypedArray())
        .redirectOutput(ProcessBuilder.Redirect.PIPE)
        .start()
        .inputStream
        .bufferedReader()
        .readText()
}
