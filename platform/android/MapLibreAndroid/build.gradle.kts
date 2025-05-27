plugins {
    alias(libs.plugins.kotlinter)
    alias(libs.plugins.dokka)
    id("com.android.library")
    id("com.jaredsburrows.license")
    kotlin("android")
    id("maplibre.download-vulkan-validation")
    id("maplibre.gradle-checkstyle")
    id("maplibre.gradle-dependencies-graph")
    id("maplibre.android-nitpick")
    id("maplibre.gradle-publish")
    id("maplibre.artifact-settings")
    id("org.maplibre.ccache-plugin")
}

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

dokka {
    moduleName.set("MapLibre Native Android")

    dokkaSourceSets {
        main {
            includes.from("Module.md")

            sourceLink {
                remoteUrl("https://github.com/maplibre/maplibre-native/tree/main/platform/android/")
                localDirectory.set(rootDir)
            }

            // TODO add externalDocumentationLinks when these get dokka or javadocs:
            // - https://github.com/maplibre/maplibre-java
            // - https://github.com/maplibre/maplibre-gestures-android
        }
    }
}

android {
    ndkVersion = Versions.ndkVersion

    defaultConfig {
        compileSdk = 34
        minSdk = 21
        targetSdk = 33
        buildConfigField("String", "GIT_REVISION_SHORT", "\"${getGitRevision()}\"")
        buildConfigField("String", "GIT_REVISION", "\"${getGitRevision(false)}\"")
        buildConfigField(
            "String",
            "MAPLIBRE_VERSION_STRING",
            "\"MapLibre Android/${project.extra["versionName"]}\""
        )
        consumerProguardFiles("proguard-rules.pro")
    }

    flavorDimensions += "renderer"
    productFlavors {
        create("drawable") {
            dimension = "renderer"
        }
        create("vulkan") {
            dimension = "renderer"
            externalNativeBuild {
                cmake {
                    arguments("-DMLN_WITH_OPENGL=OFF", "-DMLN_WITH_VULKAN=ON")
                }
            }
        }
    }

    sourceSets {
        getByName("drawable") {
            java.srcDirs("src/opengl/java/")
        }
    }

    // Build native libraries
    val nativeTargets = mutableListOf("maplibre")
    if (project.hasProperty("mapbox.with_test")) {
        nativeTargets.add("mbgl-test")
    }
    if (project.hasProperty("mapbox.with_benchmark")) {
        nativeTargets.add("mbgl-benchmark")
    }
    nativeBuild(nativeTargets)

    // Avoid naming conflicts, force usage of prefix
    resourcePrefix("maplibre_")

    sourceSets {
        getByName("main") {
            res.srcDirs("src/main/res-public")
        }
    }

    testOptions {
        unitTests {
            isReturnDefaultValues = true

            // Robolectric 4.0 required config
            // http://robolectric.org/migrating/#migrating-to-40
            isIncludeAndroidResources = true
        }
    }

    buildTypes {
        debug {
            isTestCoverageEnabled = false
            isJniDebuggable = true
        }
    }

    namespace = "org.maplibre.android"

    lint {
        checkAllWarnings = true
        disable += listOf(
            "MissingTranslation",
            "TypographyQuotes",
            "ObsoleteLintCustomCheck",
            "MissingPermission",
            "WrongThreadInterprocedural"
        )
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
    val proc = Runtime.getRuntime().exec(cmd)
    return proc.inputStream.bufferedReader().readText().trim()
}

configurations {
    getByName("implementation") {
        exclude(group = "commons-logging", module = "commons-logging")
        exclude(group = "commons-collections", module = "commons-collections")
    }
}

// apply<DownloadVulkanValidationPlugin>()

// intentionally disabled
// apply(plugin = "maplibre.jacoco-report")
