plugins {
    alias(libs.plugins.kotlinter)
    id("com.android.application")
    alias(libs.plugins.kotlinAndroid)
    alias(libs.plugins.kotlinPluginSerialization)
    id("maplibre.gradle-make")
    id("maplibre.gradle-config")
    id("maplibre.gradle-checkstyle")
    id("maplibre.gradle-lint")
}

fun obtainTestBuildType(): String {
    return if (project.hasProperty("testBuildType")) {
        project.properties["testBuildType"] as String
    } else {
        "debug"
    }
}

android {
    ndkVersion = Versions.ndkVersion

    compileSdk = 34

    defaultConfig {
        applicationId = "org.maplibre.android.testapp"
        minSdk = 23
        targetSdk = 33
        versionCode = 14
        testInstrumentationRunner = "org.maplibre.android.InstrumentationRunner"
        multiDexEnabled = true
        versionName = file("../VERSION").readText().trim()

        manifestPlaceholders["SENTRY_DSN"] = ""
        manifestPlaceholders["SENTRY_ENV"] = ""
    }

    nativeBuild(listOf("example-custom-layer"))

    packaging {
        resources.excludes += listOf("META-INF/LICENSE.txt", "META-INF/NOTICE.txt", "LICENSE.txt")
    }

    buildTypes {
        getByName("debug") {
            isJniDebuggable = true
            isDebuggable = true
            isTestCoverageEnabled = true
            isMinifyEnabled = false
            isShrinkResources = false
            proguardFiles(getDefaultProguardFile("proguard-android.txt"), "proguard-rules.pro")

            packaging {
                jniLibs {
                    keepDebugSymbols += "**/*.so"
                }
            }

            buildConfigField("String", "SENTRY_DSN", "\"" + (System.getenv("SENTRY_DSN") ?: "") + "\"")
            manifestPlaceholders["SENTRY_DSN"] = System.getenv("SENTRY_DSN") ?: ""
        }

        getByName("release") {
            isMinifyEnabled = true
            isShrinkResources = true
            proguardFiles(getDefaultProguardFile("proguard-android.txt"), "proguard-rules.pro")
            testProguardFiles("test-proguard-rules.pro")
            signingConfig = signingConfigs.getByName("debug")

            buildConfigField("String", "SENTRY_DSN", "\"" + (System.getenv("SENTRY_DSN") ?: "") + "\"")
            manifestPlaceholders["SENTRY_DSN"] = System.getenv("SENTRY_DSN") ?: ""
        }
    }

    testBuildType = obtainTestBuildType()

    flavorDimensions += "renderer"

    productFlavors {
        create("opengl") {
            dimension = "renderer"
            externalNativeBuild {
                cmake {
                    arguments("-DMLN_WITH_OPENGL=ON", "-DMLN_WITH_VULKAN=OFF")
                }
            }
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

    buildFeatures {
        viewBinding = true
        buildConfig = true
    }

    namespace = "org.maplibre.android.testapp"

    lint {
        abortOnError = false
        baseline = file("lint-baseline-local.xml")
        checkAllWarnings = true
        disable += listOf("MissingTranslation", "GoogleAppIndexingWarning", "UnpackedNativeCode", "IconDipSize", "TypographyQuotes")
        warningsAsErrors = true
    }
}

kotlin {
    jvmToolchain(17)
}

dependencies {
    implementation(project(":MapLibreAndroid"))

    implementation(libs.maplibreNavigation) {
        exclude(group = "org.maplibre.gl", module = "android-sdk")
    }

    implementation(libs.maplibreJavaTurf)

    implementation(libs.supportRecyclerView)
    implementation(libs.supportDesign)
    implementation(libs.supportConstraintLayout)
    implementation(libs.kotlinxSerializationJson)

    implementation(libs.multidex)
    implementation(libs.timber)
    implementation(libs.okhttp3)
    implementation(libs.kotlinxCoroutinesCore)
    implementation(libs.kotlinxCoroutinesAndroid)

    debugImplementation(libs.leakCanary)

    androidTestImplementation(libs.supportAnnotations)
    androidTestImplementation(libs.testRunner)
    androidTestImplementation(libs.testRules)
    androidTestImplementation(libs.testEspressoCore)
    androidTestImplementation(libs.testEspressoIntents)
    androidTestImplementation(libs.testEspressoContrib)
    androidTestImplementation(libs.testUiAutomator)
    androidTestImplementation(libs.appCenter)
    androidTestImplementation(libs.androidxTestExtJUnit)
    androidTestImplementation(libs.androidxTestCoreKtx)
    androidTestImplementation(libs.kotlinxCoroutinesTest)
}

apply<SentryConditionalPlugin>()
