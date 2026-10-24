plugins {
    id("com.android.application")
    id("org.maplibre.ccache-plugin")
}

android {
    namespace = "android.app"

    buildFeatures {
        prefab = true
    }

    defaultConfig {
        applicationId = "org.maplibre.cpp_test_runner"
        minSdk = 28
        targetSdk = 33
        compileSdk = 34

        val abi = if (project.hasProperty("maplibre.abis")) {
            project.property("maplibre.abis") as String
        } else {
            "all"
        }

        ndk {
            if (abi != "all" && abi != "none") {
                abiFilters += abi.split(" ")
            } else {
                abiFilters += listOf("armeabi-v7a", "x86", "arm64-v8a", "x86_64")
            }
        }

        externalNativeBuild {
            cmake {
                arguments("-DANDROID_STL=c++_static")
                targets += "mbgl-test-runner"
            }
        }

        testBuildType = "release"
        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        missingDimensionStrategy("renderer", "opengl")
    }

    externalNativeBuild {
        cmake {
            version = "3.24.0+"
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
            externalNativeBuild {
                cmake {
                    arguments("-DMLN_WITH_OPENGL=ON")
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
        create("webgpuDawn") {
            dimension = "renderer"
            externalNativeBuild {
                cmake {
                    arguments("-DMLN_WITH_WEBGPU=ON", "-DMLN_WEBGPU_IMPL_DAWN=ON")
                }
            }
        }
        create("webgpuWgpu") {
            dimension = "renderer"
            ndk {
                abiFilters += "arm64-v8a"
            }
            externalNativeBuild {
                cmake {
                    arguments("-DMLN_WITH_WEBGPU=ON", "-DMLN_WEBGPU_IMPL_WGPU=ON")
                }
            }
        }
    }
}

dependencies {
    implementation("androidx.constraintlayout:constraintlayout:2.1.4")
    androidTestImplementation("androidx.test.ext:junit:1.2.1")
    androidTestImplementation("androidx.test.espresso:espresso-core:3.6.1")
    androidTestImplementation("androidx.test:rules:1.6.1")
    implementation("io.github.vvb2060.ndk:curl:8.8.0")
}
