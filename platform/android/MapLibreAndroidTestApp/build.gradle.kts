plugins {
    alias(libs.plugins.kotlinter)
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
    alias(libs.plugins.kotlinPluginSerialization)
}

apply(from = "${rootDir}/gradle/native-build.gradle.kts")

fun obtainTestBuildType(): String {
    var result = "debug"

    if (project.hasProperty("testBuildType")) {
        result = project.properties["testBuildType"] as String
    }

    return result
}

android {
    defaultConfig {
        applicationId = "org.maplibre.android.testapp"
        compileSdk = 34
        minSdk = 21
        targetSdk = 33
        versionCode = 14
        versionName = "6.0.1"
        testInstrumentationRunner = "org.maplibre.android.InstrumentationRunner"
        multiDexEnabled = true
    }

    nativeBuild(listOf("example-custom-layer"))

    packagingOptions {
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
        }

        getByName("release") {
            isMinifyEnabled = true
            isShrinkResources = true
            proguardFiles(getDefaultProguardFile("proguard-android.txt"), "proguard-rules.pro")
            testProguardFiles("test-proguard-rules.pro")
            signingConfig = signingConfigs.getByName("debug")
        }
    }

    testBuildType = obtainTestBuildType()

    flavorDimensions += "renderer"
    productFlavors {
        create("legacy") {
            dimension = "renderer"
        }
        create("drawable") {
            dimension = "renderer"
        }
        create("vulkan") {
            dimension = "renderer"
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
        disable.addAll(
            listOf(
                "MissingTranslation", "GoogleAppIndexingWarning", "UnpackedNativeCode", "IconDipSize", "TypographyQuotes"
            )
        )
        warningsAsErrors = true
    }
}

kotlin {
    jvmToolchain(17)
}

dependencies {
    implementation(project(":MapLibreAndroid"))
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

apply(from = "${rootDir}/gradle/gradle-make.gradle.kts")
apply(from = "${rootDir}/gradle/gradle-config.gradle.kts")
apply(from = "${rootDir}/gradle/gradle-checkstyle.gradle.kts")
apply(from = "${rootDir}/gradle/gradle-lint.gradle.kts")
