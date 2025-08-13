plugins {
    alias(libs.plugins.kotlinter)
    id("com.android.application")
    alias(libs.plugins.kotlinAndroid)
    alias(libs.plugins.kotlinPluginSerialization)
    id("maplibre.gradle-make")
    id("maplibre.gradle-config")
    id("maplibre.gradle-checkstyle")
    id("maplibre.gradle-lint")
    alias(libs.plugins.sentry)
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
        minSdk = 21
        targetSdk = 33
        versionCode = 14
        versionName = "6.0.1"
        testInstrumentationRunner = "org.maplibre.android.InstrumentationRunner"
        multiDexEnabled = true
        manifestPlaceholders["SENTRY_DSN"] = ""
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
        disable += listOf("MissingTranslation", "GoogleAppIndexingWarning", "UnpackedNativeCode", "IconDipSize", "TypographyQuotes")
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

sentry {
    // Disables or enables debug log output, e.g. for for sentry-cli.
    // Default is disabled.
    debug.set(true)

    // The slug of the Sentry organization to use for uploading proguard mappings/source contexts.
    org.set(System.getenv("SENTRY_ORG"))

    // The slug of the Sentry project to use for uploading proguard mappings/source contexts.
    projectName.set(System.getenv("SENTRY_PROJECT"))

    // The authentication token to use for uploading proguard mappings/source contexts.
    // WARNING: Do not expose this token in your build.gradle files, but rather set an environment
    // variable and read it into this property.
    authToken.set(System.getenv("SENTRY_AUTH_TOKEN"))

    // The url of your Sentry instance. If you're using SAAS (not self hosting) you do not have to
    // set this. If you are self hosting you can set your URL here
    url = null

    // Disables or enables the handling of Proguard mapping for Sentry.
    // If enabled the plugin will generate a UUID and will take care of
    // uploading the mapping to Sentry. If disabled, all the logic
    // related to proguard mapping will be excluded.
    // Default is enabled.
    includeProguardMapping.set(true)

    // Whether the plugin should attempt to auto-upload the mapping file to Sentry or not.
    // If disabled the plugin will run a dry-run and just generate a UUID.
    // The mapping file has to be uploaded manually via sentry-cli in this case.
    // Default is enabled.
    autoUploadProguardMapping.set(true)

    // Experimental flag to turn on support for GuardSquare's tools integration (Dexguard and External Proguard).
    // If enabled, the plugin will try to consume and upload the mapping file produced by Dexguard and External Proguard.
    // Default is disabled.
    dexguardEnabled.set(false)

    // Disables or enables the automatic configuration of Native Symbols
    // for Sentry. This executes sentry-cli automatically so
    // you don't need to do it manually.
    // Default is disabled.
    uploadNativeSymbols.set(true)

    // manual upload using one of the following:
    // sentry-cli debug-files upload maplibre-native/platform/android/MapLibreAndroidTestApp
    // sentry-cli debug-files upload maplibre-native/platform/android/MapLibreAndroidTestApp/build/intermediates/merged_native_libs/<variant>/merge<variant>NativeLibs/out/lib/<abi>/libmaplibre.so

    // Whether the plugin should attempt to auto-upload the native debug symbols to Sentry or not.
    // If disabled the plugin will run a dry-run.
    // Default is enabled.
    autoUploadNativeSymbols.set(false)

    // Does or doesn't include the source code of native code for Sentry.
    // This executes sentry-cli with the --include-sources param. automatically so
    // you don't need to do it manually.
    // Default is disabled.
    includeNativeSources.set(true)

    // Generates a JVM (Java, Kotlin, etc.) source bundle and uploads your source code to Sentry.
    // This enables source context, allowing you to see your source
    // code as part of your stack traces in Sentry.
    includeSourceContext.set(true)

    // Configure additional directories to be included in the source bundle which is used for
    // source context. The directories should be specified relative to the Gradle module/project's
    // root. For example, if you have a custom source set alongside 'main', the parameter would be
    // 'src/custom/java'.
    additionalSourceDirsForSourceContext.set(emptySet())

    // Enable auto-installation of Sentry components (sentry-android SDK and okhttp, timber, fragment and compose integrations).
    // Default is enabled.
    // Only available v3.1.0 and above.
    autoInstallation {
        enabled.set(true)

        // Specifies a version of the sentry-android SDK and fragment, timber and okhttp integrations.
        //
        // This is also useful, when you have the sentry-android SDK already included into a transitive dependency/module and want to
        // align integration versions with it (if it's a direct dependency, the version will be inferred).
        //
        // NOTE: if you have a higher version of the sentry-android SDK or integrations on the classpath, this setting will have no effect
        // as Gradle will resolve it to the latest version.
        //
        // Defaults to the latest published Sentry version.
        // sentryVersion.set("8.18.0") // use default
    }

    // Disables or enables dependencies metadata reporting for Sentry.
    // If enabled, the plugin will collect external dependencies and
    // upload them to Sentry as part of events. If disabled, all the logic
    // related to the dependencies metadata report will be excluded.
    //
    // Default is enabled.
    //
    includeDependenciesReport.set(true)

    // Whether the plugin should send telemetry data to Sentry.
    // If disabled the plugin won't send telemetry data.
    // This is auto disabled if running against a self hosted instance of Sentry.
    // Default is enabled.
    telemetry.set(false)
}
