plugins {
    id("com.android.application")
}

android {
    namespace = "org.maplibre.plugins.android"
    compileSdk = 35

    defaultConfig {
        applicationId = "org.maplibre.plugindemo"
        minSdk = 23
        targetSdk = 35
        versionCode = 1
        versionName = "1.0"
    }

    flavorDimensions += "renderer"
    productFlavors {
        listOf("opengl", "vulkan", "multiBackend").forEach { renderer ->
            create(renderer) {
                dimension = "renderer"
                applicationIdSuffix = ".${renderer.lowercase()}"
                buildConfigField("boolean", "MULTI_BACKEND", (renderer == "multiBackend").toString())
            }
        }
    }

    buildFeatures {
        buildConfig = true
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
}

kotlin {
    jvmToolchain(17)
}

dependencies {
    implementation(project(":ngon-layer"))
    // Substituted with the local SDK by settings.gradle.kts, preserving renderer variants.
    implementation("org.maplibre.gl:android-sdk:0.0.0-local")
}
