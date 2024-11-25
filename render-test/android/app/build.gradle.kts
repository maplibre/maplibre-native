plugins {
    id("com.android.application")
}

android {
    ndkVersion = "27.0.12077973"

    sourceSets {
        getByName("test") {
            assets.srcDirs("../../../metrics")
        }
    }

    defaultConfig {
        applicationId = "org.maplibre.render_test_runner"
        compileSdk = 34
        minSdk = 21
        targetSdk = 33

        val abi = if (project.hasProperty("maplibre.abis")) {
            project.property("maplibre.abis") as String
        } else {
            "all"
        }

        ndk {
            if (abi != "all" && abi != "none") {
                abiFilters.addAll(abi.split(" "))
            } else {
                abiFilters.addAll(listOf("armeabi-v7a", "x86", "arm64-v8a", "x86_64"))
            }
        }

        externalNativeBuild {
            cmake {
                arguments("-DANDROID_CCACHE=ccache")
                arguments("-DANDROID_STL=c++_static")
                arguments("-DMLN_LEGACY_RENDERER=OFF", "-DMLN_DRAWABLE_RENDERER=ON")
                targets.add("mbgl-render-test-runner")
            }
        }

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    }

    externalNativeBuild {
        cmake {
            version = "3.19.0+"
            path = file("../../../CMakeLists.txt")
        }
    }

    buildTypes {
        getByName("release") {
            signingConfig = signingConfigs.getByName("debug")
        }
    }

    flavorDimensions += "renderer"

    productFlavors {
        create("opengl") {
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

    namespace = "android.app"
}

dependencies {
    implementation(libs.supportConstraintLayout)
    androidTestImplementation(libs.testRunner)
    androidTestImplementation(libs.testEspressoCore)
    androidTestImplementation(libs.testRules)
}
