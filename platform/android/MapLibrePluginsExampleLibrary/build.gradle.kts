plugins {
    id("com.android.library")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "org.maplibre.maplibrepluginsexamplelibrary"
    compileSdk = 34

    defaultConfig {
        minSdk = 23

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        consumerProguardFiles("consumer-rules.pro")
        externalNativeBuild {
            cmake {
                cppFlags("")
            }
        }
        // Specify which flavor of MapLibreAndroid to use (it has opengl/vulkan flavors)
        missingDimensionStrategy("renderer", "opengl")
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    externalNativeBuild {
        cmake {
            path("src/main/cpp/CMakeLists.txt")
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
    kotlinOptions {
        jvmTarget = "11"
    }
    ndkVersion = Versions.ndkVersion
}

dependencies {
    // Depend on MapLibreAndroid to ensure it's built first (for native library linking)
    implementation(project(":MapLibreAndroid"))

    implementation(libs.coreKtx)
    implementation(libs.appcompat)
    implementation(libs.supportDesign)
    testImplementation(libs.junit)
    androidTestImplementation(libs.testRunner)
    androidTestImplementation(libs.testEspressoCore)
}

// Ensure MapLibreAndroid's native build completes before this project's native build
afterEvaluate {
    tasks.matching { it.name.contains("externalNativeBuild") || it.name.contains("CMake") }.configureEach {
        val mapLibreNativeTasks = project(":MapLibreAndroid").tasks.matching {
            it.name.contains("externalNativeBuild") || it.name.contains("CMake")
        }
        dependsOn(mapLibreNativeTasks)
    }
}