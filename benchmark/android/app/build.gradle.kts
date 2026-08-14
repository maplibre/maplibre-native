plugins {
    id("com.android.application")
}

android {
    namespace = "org.maplibre.benchmark_runner"
    ndkVersion = "28.2.13676358"

    defaultConfig {
        applicationId = "org.maplibre.benchmark_runner"
        compileSdk = 34
        minSdk = 23
        targetSdk = 33

        val abi = providers.gradleProperty("maplibre.abis").getOrElse("all")
        ndk {
            if (abi != "all") {
                abiFilters += abi.split(" ")
            } else {
                abiFilters += listOf("armeabi-v7a", "x86", "arm64-v8a", "x86_64")
            }
        }

        externalNativeBuild {
            cmake {
                arguments += listOf(
                    "-DANDROID_CCACHE=ccache",
                    "-DANDROID_STL=c++_static",
                    "-DMLN_WITH_OPENGL=ON"
                )
                targets += "mbgl-benchmark-runner"
            }
        }

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    }

    externalNativeBuild {
        cmake {
            version = "3.24.0+"
            path = file("../../../CMakeLists.txt")
        }
    }

    buildTypes {
        getByName("release") {
            isDebuggable = true
            signingConfig = signingConfigs.getByName("debug")
        }
    }

    testBuildType = "release"
}

dependencies {
    implementation(libs.appcompat)
    implementation(libs.constraintLayout)
    androidTestImplementation(libs.testJunit)
    androidTestImplementation(libs.testEspressoCore)
    androidTestImplementation(libs.testRules)
}
