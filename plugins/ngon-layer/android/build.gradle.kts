plugins {
    id("com.android.library")
}

android {
    namespace = "org.maplibre.plugins.ngon"
    compileSdk = 35
    ndkVersion = "28.2.13676358"

    defaultConfig {
        minSdk = 23
        consumerProguardFiles("consumer-rules.pro")
        val abis = providers.gradleProperty("maplibre.abis").orElse("all").get()
        ndk {
            abiFilters += if (abis == "all") {
                listOf("armeabi-v7a", "arm64-v8a", "x86", "x86_64")
            } else {
                abis.split(" ")
            }
        }
        externalNativeBuild.cmake.arguments += "-DANDROID_STL=c++_static"
    }

    flavorDimensions += "renderer"
    productFlavors {
        listOf("opengl", "vulkan", "multiBackend").forEach { renderer ->
            create(renderer) {
                dimension = "renderer"
                buildConfigField("boolean", "MULTI_BACKEND", (renderer == "multiBackend").toString())
            }
        }
    }

    buildFeatures {
        prefab = true
        buildConfig = true
    }
    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.24.0+"
        }
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
    implementation("org.maplibre.gl:android-sdk:0.0.0-local")
}
